/*
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2010, 2012 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2012 Ralf Jung <ralfjung-e@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kshareddatacache.h"
#include "kcoreaddons_debug.h"
#include "ksdclock_p.h"
#include "ksdcmemory_p.h"

#include "kshareddatacache_p.h" // Various auxiliary support code

#include <QStandardPaths>
#include <qplatformdefs.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QRandomGenerator>

#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>

// The per-instance private data, such as map size, whether
// attached or not, pointer to shared memory, etc.
class Q_DECL_HIDDEN KSharedDataCache::Private
{
public:
    Private(const QString &name, unsigned defaultCacheSize, unsigned expectedItemSize)
        : m_cacheName(name)
        , shm(nullptr)
        , m_lock()
        , m_mapSize(0)
        , m_defaultCacheSize(defaultCacheSize)
        , m_expectedItemSize(expectedItemSize)
        , m_expectedType(LOCKTYPE_INVALID)
    {
        mapSharedMemory();
    }

    // Put the cache in a condition to be able to call mapSharedMemory() by
    // completely detaching from shared memory (such as to respond to an
    // unrecoverable error).
    // m_mapSize must already be set to the amount of memory mapped to shm.
    void detachFromSharedMemory()
    {
        // The lock holds a reference into shared memory, so this must be
        // cleared before shm is removed.
        m_lock.reset();

        if (shm && 0 != ::munmap(shm, m_mapSize)) {
            qCCritical(KCOREADDONS_DEBUG) << "Unable to unmap shared memory segment" << static_cast<void *>(shm) << ":" << ::strerror(errno);
        }

        shm = nullptr;
        m_mapSize = 0;
    }

    // This function does a lot of the important work, attempting to connect to shared
    // memory, a private anonymous mapping if that fails, and failing that, nothing (but
    // the cache remains "valid", we just don't actually do anything).
    void mapSharedMemory()
    {
        // 0-sized caches are fairly useless.
        unsigned cacheSize = qMax(m_defaultCacheSize, uint(SharedMemory::MINIMUM_CACHE_SIZE));
        unsigned pageSize = SharedMemory::equivalentPageSize(m_expectedItemSize);

        // Ensure that the cache is sized such that there is a minimum number of
        // pages available. (i.e. a cache consisting of only 1 page is fairly
        // useless and probably crash-prone).
        cacheSize = qMax(pageSize * 256, cacheSize);

        // The m_cacheName is used to find the file to store the cache in.
        const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
        QString cacheName = cacheDir + QLatin1String("/") + m_cacheName + QLatin1String(".kcache");
        QFile file(cacheName);
        QFileInfo fileInfo(file);
        if (!QDir().mkpath(fileInfo.absolutePath())) {
            return;
        }

        // The basic idea is to open the file that we want to map into shared
        // memory, and then actually establish the mapping. Once we have mapped the
        // file into shared memory we can close the file handle, the mapping will
        // still be maintained (unless the file is resized to be shorter than
        // expected, which we don't handle yet :-( )

        // size accounts for the overhead over the desired cacheSize
        uint size = SharedMemory::totalSize(cacheSize, pageSize);
        void *mapAddress = MAP_FAILED;

        if (size < cacheSize) {
            qCCritical(KCOREADDONS_DEBUG) << "Asked for a cache size less than requested size somehow -- Logic Error :(";
            return;
        }

        // We establish the shared memory mapping here, only if we will have appropriate
        // mutex support (systemSupportsProcessSharing), then we:
        // Open the file and resize to some sane value if the file is too small.
        if (file.open(QIODevice::ReadWrite) && (file.size() >= size || (ensureFileAllocated(file.handle(), size) && file.resize(size)))) {
            // Use mmap directly instead of QFile::map since the QFile (and its
            // shared mapping) will disappear unless we hang onto the QFile for no
            // reason (see the note below, we don't care about the file per se...)
            mapAddress = QT_MMAP(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, file.handle(), 0);

            // So... it is possible that someone else has mapped this cache already
            // with a larger size. If that's the case we need to at least match
            // the size to be able to access every entry, so fixup the mapping.
            if (mapAddress != MAP_FAILED) {
                SharedMemory *mapped = reinterpret_cast<SharedMemory *>(mapAddress);

                // First make sure that the version of the cache on disk is
                // valid.  We also need to check that version != 0 to
                // disambiguate against an uninitialized cache.
                if (mapped->version != SharedMemory::PIXMAP_CACHE_VERSION && mapped->version > 0) {
                    qCWarning(KCOREADDONS_DEBUG) << "Deleting wrong version of cache" << cacheName;

                    // CAUTION: Potentially recursive since the recovery
                    // involves calling this function again.
                    m_mapSize = size;
                    shm = mapped;
                    recoverCorruptedCache();
                    return;
                } else if (mapped->cacheSize > cacheSize) {
                    // This order is very important. We must save the cache size
                    // before we remove the mapping, but unmap before overwriting
                    // the previous mapping size...
                    cacheSize = mapped->cacheSize;
                    unsigned actualPageSize = mapped->cachePageSize();
                    ::munmap(mapAddress, size);
                    size = SharedMemory::totalSize(cacheSize, actualPageSize);
                    mapAddress = QT_MMAP(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, file.handle(), 0);
                }
            }
        }

        // We could be here without the mapping established if:
        // 1) Process-shared synchronization is not supported, either at compile or run time,
        // 2) Unable to open the required file.
        // 3) Unable to resize the file to be large enough.
        // 4) Establishing the mapping failed.
        // 5) The mapping succeeded, but the size was wrong and we were unable to map when
        //    we tried again.
        // 6) The incorrect version of the cache was detected.
        // 7) The file could be created, but posix_fallocate failed to commit it fully to disk.
        // In any of these cases, attempt to fallback to the
        // better-supported anonymous private page style of mmap. This memory won't
        // be shared, but our code will still work the same.
        // NOTE: We never use the on-disk representation independently of the
        // shared memory. If we don't get shared memory the disk info is ignored,
        // if we do get shared memory we never look at disk again.
        if (mapAddress == MAP_FAILED) {
            qCWarning(KCOREADDONS_DEBUG) << "Failed to establish shared memory mapping, will fallback"
                                         << "to private memory -- memory usage will increase";

            mapAddress = QT_MMAP(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        }

        // Well now we're really hosed. We can still work, but we can't even cache
        // data.
        if (mapAddress == MAP_FAILED) {
            qCCritical(KCOREADDONS_DEBUG) << "Unable to allocate shared memory segment for shared data cache" << cacheName << "of size" << cacheSize;
            return;
        }

        m_mapSize = size;

        // We never actually construct shm, but we assign it the same address as the
        // shared memory we just mapped, so effectively shm is now a SharedMemory that
        // happens to be located at mapAddress.
        shm = reinterpret_cast<SharedMemory *>(mapAddress);

        // If we were first to create this memory map, all data will be 0.
        // Therefore if ready == 0 we're not initialized.  A fully initialized
        // header will have ready == 2.  Why?
        // Because 0 means "safe to initialize"
        //         1 means "in progress of initing"
        //         2 means "ready"
        uint usecSleepTime = 8; // Start by sleeping for 8 microseconds
        while (shm->ready.loadRelaxed() != 2) {
            if (Q_UNLIKELY(usecSleepTime >= (1 << 21))) {
                // Didn't acquire within ~8 seconds?  Assume an issue exists
                qCCritical(KCOREADDONS_DEBUG) << "Unable to acquire shared lock, is the cache corrupt?";

                file.remove(); // Unlink the cache in case it's corrupt.
                detachFromSharedMemory();
                return; // Fallback to QCache (later)
            }

            if (shm->ready.testAndSetAcquire(0, 1)) {
                if (!shm->performInitialSetup(cacheSize, pageSize)) {
                    qCCritical(KCOREADDONS_DEBUG) << "Unable to perform initial setup, this system probably "
                                                     "does not really support process-shared pthreads or "
                                                     "semaphores, even though it claims otherwise.";

                    file.remove();
                    detachFromSharedMemory();
                    return;
                }
            } else {
                usleep(usecSleepTime); // spin

                // Exponential fallback as in Ethernet and similar collision resolution methods
                usecSleepTime *= 2;
            }
        }

        m_expectedType = shm->shmLock.type;
        m_lock = std::unique_ptr<KSDCLock>(createLockFromId(m_expectedType, shm->shmLock));
        bool isProcessSharingSupported = false;

        if (!m_lock->initialize(isProcessSharingSupported)) {
            qCCritical(KCOREADDONS_DEBUG) << "Unable to setup shared cache lock, although it worked when created.";
            detachFromSharedMemory();
        }
    }

    // Called whenever the cache is apparently corrupt (for instance, a timeout trying to
    // lock the cache). In this situation it is safer just to destroy it all and try again.
    void recoverCorruptedCache()
    {
        KSharedDataCache::deleteCache(m_cacheName);

        detachFromSharedMemory();

        // Do this even if we weren't previously cached -- it might work now.
        mapSharedMemory();
    }

    // This should be called for any memory access to shared memory. This
    // function will verify that the bytes [base, base+accessLength) are
    // actually mapped to d->shm. The cache itself may have incorrect cache
    // page sizes, incorrect cache size, etc. so this function should be called
    // despite the cache data indicating it should be safe.
    //
    // If the access is /not/ safe then a KSDCCorrupted exception will be
    // thrown, so be ready to catch that.
    void verifyProposedMemoryAccess(const void *base, unsigned accessLength) const
    {
        quintptr startOfAccess = reinterpret_cast<quintptr>(base);
        quintptr startOfShm = reinterpret_cast<quintptr>(shm);

        if (Q_UNLIKELY(startOfAccess < startOfShm)) {
            throw KSDCCorrupted();
        }

        quintptr endOfShm = startOfShm + m_mapSize;
        quintptr endOfAccess = startOfAccess + accessLength;

        // Check for unsigned integer wraparound, and then
        // bounds access
        if (Q_UNLIKELY((endOfShm < startOfShm) || (endOfAccess < startOfAccess) || (endOfAccess > endOfShm))) {
            throw KSDCCorrupted();
        }
    }

    bool lock() const
    {
        if (Q_LIKELY(shm && shm->shmLock.type == m_expectedType)) {
            return m_lock->lock();
        }

        // No shm or wrong type --> corrupt!
        throw KSDCCorrupted();
    }

    void unlock() const
    {
        m_lock->unlock();
    }

    class CacheLocker
    {
        mutable Private *d;

        bool cautiousLock()
        {
            int lockCount = 0;

            // Locking can fail due to a timeout. If it happens too often even though
            // we're taking corrective action assume there's some disastrous problem
            // and give up.
            while (!d->lock() && !isLockedCacheSafe()) {
                d->recoverCorruptedCache();

                if (!d->shm) {
                    qCWarning(KCOREADDONS_DEBUG) << "Lost the connection to shared memory for cache" << d->m_cacheName;
                    return false;
                }

                if (lockCount++ > 4) {
                    qCCritical(KCOREADDONS_DEBUG) << "There is a very serious problem with the KDE data cache" << d->m_cacheName
                                                  << "giving up trying to access cache.";
                    d->detachFromSharedMemory();
                    return false;
                }
            }

            return true;
        }

        // Runs a quick battery of tests on an already-locked cache and returns
        // false as soon as a sanity check fails. The cache remains locked in this
        // situation.
        bool isLockedCacheSafe() const
        {
            // Note that cachePageSize() itself runs a check that can throw.
            uint testSize = SharedMemory::totalSize(d->shm->cacheSize, d->shm->cachePageSize());

            if (Q_UNLIKELY(d->m_mapSize != testSize)) {
                return false;
            }
            if (Q_UNLIKELY(d->shm->version != SharedMemory::PIXMAP_CACHE_VERSION)) {
                return false;
            }
            switch (d->shm->evictionPolicy.loadRelaxed()) {
            case NoEvictionPreference: // fallthrough
            case EvictLeastRecentlyUsed: // fallthrough
            case EvictLeastOftenUsed: // fallthrough
            case EvictOldest:
                break;
            default:
                return false;
            }

            return true;
        }

    public:
        CacheLocker(const Private *_d)
            : d(const_cast<Private *>(_d))
        {
            if (Q_UNLIKELY(!d || !d->shm || !cautiousLock())) {
                d = nullptr;
            }
        }

        ~CacheLocker()
        {
            if (d && d->shm) {
                d->unlock();
            }
        }

        CacheLocker(const CacheLocker &) = delete;
        CacheLocker &operator=(const CacheLocker &) = delete;

        bool failed() const
        {
            return !d || d->shm == nullptr;
        }
    };

    QString m_cacheName;
    SharedMemory *shm;
    std::unique_ptr<KSDCLock> m_lock;
    uint m_mapSize;
    uint m_defaultCacheSize;
    uint m_expectedItemSize;
    SharedLockId m_expectedType;
};

KSharedDataCache::KSharedDataCache(const QString &cacheName, unsigned defaultCacheSize, unsigned expectedItemSize)
    : d(nullptr)
{
    try {
        d = new Private(cacheName, defaultCacheSize, expectedItemSize);
    } catch (KSDCCorrupted) {
        KSharedDataCache::deleteCache(cacheName);

        // Try only once more
        try {
            d = new Private(cacheName, defaultCacheSize, expectedItemSize);
        } catch (KSDCCorrupted) {
            qCCritical(KCOREADDONS_DEBUG) << "Even a brand-new cache starts off corrupted, something is"
                                          << "seriously wrong. :-(";
            d = nullptr; // Just in case
        }
    }
}

KSharedDataCache::~KSharedDataCache()
{
    // Note that there is no other actions required to separate from the
    // shared memory segment, simply unmapping is enough. This makes things
    // *much* easier so I'd recommend maintaining this ideal.
    if (!d) {
        return;
    }

    if (d->shm) {
#ifdef KSDC_MSYNC_SUPPORTED
        ::msync(d->shm, d->m_mapSize, MS_INVALIDATE | MS_ASYNC);
#endif
        ::munmap(d->shm, d->m_mapSize);
    }

    // Do not delete d->shm, it was never constructed, it's just an alias.
    d->shm = nullptr;

    delete d;
}

bool KSharedDataCache::insert(const QString &key, const QByteArray &data)
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return false;
        }

        QByteArray encodedKey = key.toUtf8();
        uint keyHash = SharedMemory::generateHash(encodedKey);
        uint position = keyHash % d->shm->indexTableSize();

        // See if we're overwriting an existing entry.
        IndexTableEntry *indices = d->shm->indexTable();

        // In order to avoid the issue of a very long-lived cache having items
        // with a use count of 1 near-permanently, we attempt to artifically
        // reduce the use count of long-lived items when there is high load on
        // the cache. We do this randomly, with a weighting that makes the event
        // impossible if load < 0.5, and guaranteed if load >= 0.96.
        const static double startCullPoint = 0.5l;
        const static double mustCullPoint = 0.96l;

        // cacheAvail is in pages, cacheSize is in bytes.
        double loadFactor = 1.0 - (1.0l * d->shm->cacheAvail * d->shm->cachePageSize() / d->shm->cacheSize);
        bool cullCollisions = false;

        if (Q_UNLIKELY(loadFactor >= mustCullPoint)) {
            cullCollisions = true;
        } else if (loadFactor > startCullPoint) {
            const int tripWireValue = RAND_MAX * (loadFactor - startCullPoint) / (mustCullPoint - startCullPoint);
            if (QRandomGenerator::global()->bounded(RAND_MAX) >= tripWireValue) {
                cullCollisions = true;
            }
        }

        // In case of collisions in the index table (i.e. identical positions), use
        // quadratic chaining to attempt to find an empty slot. The equation we use
        // is:
        // position = (hash + (i + i*i) / 2) % size, where i is the probe number.
        uint probeNumber = 1;
        while (indices[position].useCount > 0 && probeNumber < SharedMemory::MAX_PROBE_COUNT) {
            // If we actually stumbled upon an old version of the key we are
            // overwriting, then use that position, do not skip over it.

            if (Q_UNLIKELY(indices[position].fileNameHash == keyHash)) {
                break;
            }

            // If we are "culling" old entries, see if this one is old and if so
            // reduce its use count. If it reduces to zero then eliminate it and
            // use its old spot.

            if (cullCollisions && (::time(nullptr) - indices[position].lastUsedTime) > 60) {
                indices[position].useCount >>= 1;
                if (indices[position].useCount == 0) {
                    qCDebug(KCOREADDONS_DEBUG) << "Overwriting existing old cached entry due to collision.";
                    d->shm->removeEntry(position); // Remove it first
                    break;
                }
            }

            position = (keyHash + (probeNumber + probeNumber * probeNumber) / 2) % d->shm->indexTableSize();
            probeNumber++;
        }

        if (indices[position].useCount > 0 && indices[position].firstPage >= 0) {
            qCDebug(KCOREADDONS_DEBUG) << "Overwriting existing cached entry due to collision.";
            d->shm->removeEntry(position); // Remove it first
        }

        // Data will be stored as fileNamefoo\0PNGimagedata.....
        // So total size required is the length of the encoded file name + 1
        // for the trailing null, and then the length of the image data.
        uint fileNameLength = 1 + encodedKey.length();
        uint requiredSize = fileNameLength + data.size();
        uint pagesNeeded = SharedMemory::intCeil(requiredSize, d->shm->cachePageSize());
        uint firstPage(-1);

        if (pagesNeeded >= d->shm->pageTableSize()) {
            qCWarning(KCOREADDONS_DEBUG) << key << "is too large to be cached.";
            return false;
        }

        // If the cache has no room, or the fragmentation is too great to find
        // the required number of consecutive free pages, take action.
        if (pagesNeeded > d->shm->cacheAvail || (firstPage = d->shm->findEmptyPages(pagesNeeded)) >= d->shm->pageTableSize()) {
            // If we have enough free space just defragment
            uint freePagesDesired = 3 * qMax(1u, pagesNeeded / 2);

            if (d->shm->cacheAvail > freePagesDesired) {
                // TODO: How the hell long does this actually take on real
                // caches?
                d->shm->defragment();
                firstPage = d->shm->findEmptyPages(pagesNeeded);
            } else {
                // If we already have free pages we don't want to remove a ton
                // extra. However we can't rely on the return value of
                // removeUsedPages giving us a good location since we're not
                // passing in the actual number of pages that we need.
                d->shm->removeUsedPages(qMin(2 * freePagesDesired, d->shm->pageTableSize()) - d->shm->cacheAvail);
                firstPage = d->shm->findEmptyPages(pagesNeeded);
            }

            if (firstPage >= d->shm->pageTableSize() || d->shm->cacheAvail < pagesNeeded) {
                qCCritical(KCOREADDONS_DEBUG) << "Unable to free up memory for" << key;
                return false;
            }
        }

        // Update page table
        PageTableEntry *table = d->shm->pageTable();
        for (uint i = 0; i < pagesNeeded; ++i) {
            table[firstPage + i].index = position;
        }

        // Update index
        indices[position].fileNameHash = keyHash;
        indices[position].totalItemSize = requiredSize;
        indices[position].useCount = 1;
        indices[position].addTime = ::time(nullptr);
        indices[position].lastUsedTime = indices[position].addTime;
        indices[position].firstPage = firstPage;

        // Update cache
        d->shm->cacheAvail -= pagesNeeded;

        // Actually move the data in place
        void *dataPage = d->shm->page(firstPage);
        if (Q_UNLIKELY(!dataPage)) {
            throw KSDCCorrupted();
        }

        // Verify it will all fit
        d->verifyProposedMemoryAccess(dataPage, requiredSize);

        // Cast for byte-sized pointer arithmetic
        uchar *startOfPageData = reinterpret_cast<uchar *>(dataPage);
        ::memcpy(startOfPageData, encodedKey.constData(), fileNameLength);
        ::memcpy(startOfPageData + fileNameLength, data.constData(), data.size());

        return true;
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return false;
    }
}

bool KSharedDataCache::find(const QString &key, QByteArray *destination) const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return false;
        }

        // Search in the index for our data, hashed by key;
        QByteArray encodedKey = key.toUtf8();
        qint32 entry = d->shm->findNamedEntry(encodedKey);

        if (entry >= 0) {
            const IndexTableEntry *header = &d->shm->indexTable()[entry];
            const void *resultPage = d->shm->page(header->firstPage);
            if (Q_UNLIKELY(!resultPage)) {
                throw KSDCCorrupted();
            }

            d->verifyProposedMemoryAccess(resultPage, header->totalItemSize);

            header->useCount++;
            header->lastUsedTime = ::time(nullptr);

            // Our item is the key followed immediately by the data, so skip
            // past the key.
            const char *cacheData = reinterpret_cast<const char *>(resultPage);
            cacheData += encodedKey.size();
            cacheData++; // Skip trailing null -- now we're pointing to start of data

            if (destination) {
                *destination = QByteArray(cacheData, header->totalItemSize - encodedKey.size() - 1);
            }

            return true;
        }
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
    }

    return false;
}

void KSharedDataCache::clear()
{
    try {
        Private::CacheLocker lock(d);

        if (!lock.failed()) {
            d->shm->clear();
        }
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
    }
}

bool KSharedDataCache::contains(const QString &key) const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return false;
        }

        return d->shm->findNamedEntry(key.toUtf8()) >= 0;
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return false;
    }
}

void KSharedDataCache::deleteCache(const QString &cacheName)
{
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/") + cacheName + QLatin1String(".kcache");

    // Note that it is important to simply unlink the file, and not truncate it
    // smaller first to avoid SIGBUS errors and similar with shared memory
    // attached to the underlying inode.
    qCDebug(KCOREADDONS_DEBUG) << "Removing cache at" << cachePath;
    QFile::remove(cachePath);
}

unsigned KSharedDataCache::totalSize() const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return 0u;
        }

        return d->shm->cacheSize;
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return 0u;
    }
}

unsigned KSharedDataCache::freeSize() const
{
    try {
        Private::CacheLocker lock(d);
        if (lock.failed()) {
            return 0u;
        }

        return d->shm->cacheAvail * d->shm->cachePageSize();
    } catch (KSDCCorrupted) {
        d->recoverCorruptedCache();
        return 0u;
    }
}

KSharedDataCache::EvictionPolicy KSharedDataCache::evictionPolicy() const
{
    if (d && d->shm) {
        return static_cast<EvictionPolicy>(d->shm->evictionPolicy.fetchAndAddAcquire(0));
    }

    return NoEvictionPreference;
}

void KSharedDataCache::setEvictionPolicy(EvictionPolicy newPolicy)
{
    if (d && d->shm) {
        d->shm->evictionPolicy.fetchAndStoreRelease(static_cast<int>(newPolicy));
    }
}

unsigned KSharedDataCache::timestamp() const
{
    if (d && d->shm) {
        return static_cast<unsigned>(d->shm->cacheTimestamp.fetchAndAddAcquire(0));
    }

    return 0;
}

void KSharedDataCache::setTimestamp(unsigned newTimestamp)
{
    if (d && d->shm) {
        d->shm->cacheTimestamp.fetchAndStoreRelease(static_cast<int>(newTimestamp));
    }
}
