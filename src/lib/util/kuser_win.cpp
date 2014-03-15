/*
 *  KUser - represent a user/account (Windows)
 *  Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
 *  Copyright (C) 2014 Alex Richardson <arichardson.kde@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kuser.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#include <memory> // unique_ptr
#include <type_traits>

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x600 //Vista for SHGetKnownFolderPath
#include <qt_windows.h>
#include <LM.h> //Net*
#include <sddl.h> //ConvertSidToStringSidW
#include <Shlobj.h>

static const auto netApiBufferDeleter = [](void* buffer) {
    if (buffer) {
        NetApiBufferFree(buffer);
    }
};

template<typename T>
class ScopedNetApiBuffer : public std::unique_ptr<T, decltype(netApiBufferDeleter)> {
public:
    inline explicit ScopedNetApiBuffer(T* data)
        : std::unique_ptr<T, decltype(netApiBufferDeleter)>(data, netApiBufferDeleter) {}
};

const auto handleCloser = [](HANDLE h) {
    if (h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
    }
};
typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, decltype(handleCloser)> ScopedHANDLE;

/** Make sure the NetApi functions are called with the correct level argument (for template functions)
* This argument can be retrieved by using NetApiTypeInfo<T>::level. In order to do so the type must be
* registered by writing e.g. NETAPI_TYPE_INFO(GROUP_INFO, 0) for GROUP_INFO_0
*/
template<typename T> struct NetApiTypeInfo {};
#define NETAPI_TYPE_INFO(prefix, n) template<> struct NetApiTypeInfo<prefix##_##n> { enum { level = n }; };
NETAPI_TYPE_INFO(GROUP_INFO, 0);
NETAPI_TYPE_INFO(GROUP_INFO, 3);
NETAPI_TYPE_INFO(USER_INFO, 0);
NETAPI_TYPE_INFO(USER_INFO, 11);
NETAPI_TYPE_INFO(GROUP_USERS_INFO, 0);

// T must be a USER_INFO_* structure
template<typename T>
ScopedNetApiBuffer<T> getUserInfo(LPCWSTR server, const QString& userName, NET_API_STATUS* errCode)
{
    LPBYTE userInfoTmp = nullptr;
    // if level = 11 a USER_INFO_11 structure gets filled in and allocated by NetUserGetInfo(), etc.
    NET_API_STATUS status = NetUserGetInfo(server, (LPCWSTR)userName.utf16(), NetApiTypeInfo<T>::level, &userInfoTmp);
    if (status != NERR_Success) {
        userInfoTmp = nullptr;
    }
    if (errCode) {
        *errCode = status;
    }
    return ScopedNetApiBuffer<T>((T*)userInfoTmp);
}

class KUser::Private : public QSharedData
{
    typedef QExplicitlySharedDataPointer<Private> Ptr;
    Private() : isAdmin(false) {}
    //takes ownership over userInfo_
    Private(KUserId uid, const QString& loginName, const QString& fullName,
        const QString& domain, const QString& homeDir, bool isAdmin)
        : uid(uid), loginName(loginName), fullName(fullName), domain(domain), homeDir(homeDir), isAdmin(isAdmin)
    {
        Q_ASSERT(uid.isValid());
    }
    static QString guessHomeDir(const QString& username, KUserId uid)
    {
        // usri11_home_dir/usri4_home_dir is often empty
        // check whether it is the homedir for the current user and if not then fall back to "<user profiles dir>\<user name>"
        if (uid == KUserId::currentUserId()) {
            return QDir::homePath();
        }
        QString homeDir;
        WCHAR* profileDirPath; // must be freed using CoTaskMemFree()
        //TODO: what does KF_FLAG_SIMPLE_IDLIST do?
        HRESULT result = SHGetKnownFolderPath(FOLDERID_UserProfiles, KF_FLAG_DONT_VERIFY, nullptr, &profileDirPath);
        if (result == S_OK) {
            // This might not be correct: e.g. with local user and domain user with same
            // In that case it could be C:\Users\Foo (local user) vs C:\Users\Foo.DOMAIN (domain user)
            // However it is still much better than the previous code which just returned the current users home dir
            homeDir = QString::fromWCharArray(profileDirPath) + QLatin1Char('\\') + username;
            CoTaskMemFree(profileDirPath);
        }
        return homeDir;
    }
public:
    static Ptr sharedNull;
    KUserId uid;
    QString loginName;
    QString fullName;
    QString domain;
    QString homeDir;
    bool isAdmin;

    /** Creates a user info from a SID (never returns null) */
    static Ptr create(KUserId sid)
    {
        if (!sid.isValid()) {
            return sharedNull;
        }
        // now find the fully qualified name for the user
        DWORD nameBufferLen = UNLEN + 1;
        WCHAR nameBuffer[UNLEN + 1];
        DWORD domainBufferLen = UNLEN + 1;
        WCHAR domainBuffer[UNLEN + 1];
        SID_NAME_USE use;
        if (!LookupAccountSidW(nullptr, sid.nativeId(), nameBuffer, &nameBufferLen, domainBuffer, &domainBufferLen, &use)) {
            qWarning() << "Could not lookup user " << sid.toString() << "error =" << GetLastError();
            return sharedNull;
        }
        QString loginName = QString::fromWCharArray(nameBuffer);
        QString domainName = QString::fromWCharArray(domainBuffer);
        if (use != SidTypeUser && use != SidTypeDeletedAccount) {
            qWarning().nospace() << "SID for " << domainName << "\\" << loginName << " (" << sid.toString()
                << ") is not of type user (" << SidTypeUser << " or " << SidTypeDeletedAccount
                << "). Got type " << use << " instead.";
            return sharedNull;
        }
        // now get the server name to query (could be null for local machine)
        LPWSTR servernameTmp = nullptr;
        NET_API_STATUS status = NetGetAnyDCName(nullptr, 0, (LPBYTE*)&servernameTmp);
        if (status != NERR_Success) {
            // this always fails on my desktop system, don't spam the output
            // qDebug("NetGetAnyDCName failed with error %d", status);
        }
        ScopedNetApiBuffer<WCHAR> servername(servernameTmp);

        // must NOT pass the qualified name ("domain\user") here or lookup fails -> just the name
        ScopedNetApiBuffer<USER_INFO_11> userInfo11(getUserInfo<USER_INFO_11>(servername.get(), loginName, &status));
        if (!userInfo11) {
            qDebug().nospace() << "Could not get information for user " << domainName << "\\" << loginName
                << ": error code = " << status;
            return sharedNull;
        }
        QString fullName = QString::fromWCharArray(userInfo11->usri11_full_name);
        QString homeDir = QString::fromWCharArray(userInfo11->usri11_home_dir);
        bool isAdmin = userInfo11->usri11_priv == USER_PRIV_ADMIN;
        if (homeDir.isEmpty()) {
            homeDir = guessHomeDir(loginName, sid);
        }
        return Ptr(new Private(sid, loginName, fullName, domainName, homeDir, isAdmin));
    }
};

KUser::Private::Ptr KUser::Private::sharedNull(new KUser::Private());

KUser::KUser(UIDMode mode)
{
    if (mode == UseEffectiveUID) {
        d = Private::create(KUserId::currentEffectiveUserId());
    } else if (mode == UseRealUserID) {
        d = Private::create(KUserId::currentUserId());
    }
    else {
        d = Private::sharedNull;
    }
}

KUser::KUser(K_UID uid)
    : d(Private::create(KUserId(uid)))
{
}

KUser::KUser(KUserId uid)
    : d(Private::create(uid))
{
}

KUser::KUser(const QString &name)
    : d(Private::create(KUserId::fromName(name)))
{
}

KUser::KUser(const char *name)
    : d(Private::create(KUserId::fromName(QString::fromLocal8Bit(name))))
{
}

KUser::KUser(const KUser &user)
    : d(user.d)
{
}

KUser &KUser::operator=(const KUser &user)
{
    d = user.d;
    return *this;
}

bool KUser::operator==(const KUser &user) const
{
    return isValid() && d->uid == user.d->uid;
}

bool KUser::operator !=(const KUser &user) const
{
    return !operator==(user);
}

bool KUser::isValid() const
{
    return d->uid.isValid();
}

bool KUser::isSuperUser() const
{
    return d->isAdmin;
}

QString KUser::loginName() const
{
    return d->loginName;
}

QString KUser::homeDir() const
{
    return d->homeDir;
}

/* See MSDN (http://msdn.microsoft.com/en-us/library/windows/desktop/bb776892%28v=vs.85%29.aspx)
 *
 * The user's tile image is stored in the the current temporary directory 
 * (%TEMP%, usually C:\Users\Alex\AppData\Local\Temp) as <username>.bmp.
 * Any slash characters (\) are converted to plus sign characters (+).
 * For example, DOMAIN\user is converted to DOMAIN+user.
 */
static inline QString tileImageName(const QString& user) {
    QString ret = user;
    ret.replace(QLatin1Char('\\'), QLatin1Char('+'));
    return ret + QStringLiteral(".bmp");
}

QString KUser::faceIconPath() const
{
    // try name with domain first, then fallback to logon name
    const QString nameWithDomain = d->domain + QLatin1Char('\\') + d->loginName;
    QString imagePath = QStandardPaths::locate(QStandardPaths::TempLocation, tileImageName(nameWithDomain));
    if (imagePath.isEmpty()) {
        imagePath = QStandardPaths::locate(QStandardPaths::TempLocation, tileImageName(loginName()));
    }
    return imagePath;
}

QString KUser::shell() const
{
    return QString::fromLatin1("cmd.exe");
}

KUserId KUser::userId() const
{
    return d->uid;
}


QVariant KUser::property(UserProperty which) const
{
    if (which == FullName) {
        return QVariant(d->fullName);
    }

    return QVariant();
}

KUser::~KUser()
{
}

class KUserGroup::Private : public QSharedData
{
public:
    QString name;
    KGroupId gid;
    Private() {}
    Private(const QString &name, KGroupId id)
        : name(name), gid(id)
    {
        if (!name.isEmpty()) {
            PBYTE groupInfoTmp = nullptr;
            NET_API_STATUS status = NetGroupGetInfo(nullptr, (LPCWSTR)name.utf16(), 0, &groupInfoTmp);
            // must always be freed, even on error
            ScopedNetApiBuffer<GROUP_INFO_0> groupInfo((GROUP_INFO_0*)groupInfoTmp);
            if (status != NERR_Success) {
                qWarning() << "Failed to find group with name" << name << "error =" << status;
                groupInfo.reset();
            }
            if (!id.isValid()) {
                gid = KGroupId::fromName(name);
            }
        }
    }
};

KUserGroup::KUserGroup(const QString &_name)
    : d(new Private(_name, KGroupId()))
{
}

KUserGroup::KUserGroup(const char *_name)
    : d(new Private(QLatin1String(_name), KGroupId()))
{
}

KUserGroup::KUserGroup(KGroupId gid)
{
    DWORD bufferLen = UNLEN + 1;
    WCHAR buffer[UNLEN + 1];
    DWORD domainBufferLen = UNLEN + 1;
    WCHAR domainBuffer[UNLEN + 1];
    SID_NAME_USE eUse;
    QString name;
    if (gid.isValid() && LookupAccountSidW(NULL, gid.nativeId(), buffer, &bufferLen, domainBuffer, &domainBufferLen, &eUse)) {
        if (eUse == SidTypeGroup || eUse == SidTypeWellKnownGroup) {
            name = QString::fromWCharArray(buffer);
        } else {
            qWarning() << QString::fromWCharArray(buffer) << "is not a group, SID type is" << eUse;
        }
    }
    d = new Private(name, gid);

}

KUserGroup::KUserGroup(const KUserGroup &group)
    : d(group.d)
{
}

KUserGroup &KUserGroup::operator =(const KUserGroup &group)
{
    d = group.d;
    return *this;
}

bool KUserGroup::operator==(const KUserGroup &group) const
{
    return isValid() && d->gid == group.d->gid && d->name == group.d->name;
}

bool KUserGroup::operator!=(const KUserGroup &group) const
{
    return !operator==(group);
}

bool KUserGroup::isValid() const
{
    return d->gid.isValid() && !d->name.isEmpty();
}

QString KUserGroup::name() const
{
    return d->name;
}

KGroupId KUserGroup::groupId() const
{
    return d->gid;
}

KUserGroup::~KUserGroup()
{
}

//enumeration functions
/** simplify calling the Net*Enum functions to prevent copy and paste for allUsers(), allUserNames(), allGroups(), allGroupNames()
* @tparam T The type that is enumerated (e.g. USER_INFO_11) Must be registered using NETAPI_TYPE_INFO.
* @param callback Callback for each listed object. Signature: void(const T&)
* @param enumFunc This function enumerates the data using a Net* function.
* It will be called in a loop as long as it returns ERROR_MORE_DATA.
*
*/
template<class T, class Callback, class EnumFunction>
static void netApiEnumerate(uint maxCount, Callback callback, EnumFunction enumFunc) {
    NET_API_STATUS nStatus = NERR_Success;
    quint64 resumeHandle = 0;
    uint total = 0;
    int level = NetApiTypeInfo<T>::level;
    do {
        LPBYTE buffer = nullptr;
        DWORD entriesRead = 0;
        DWORD totalEntries = 0;
        nStatus = enumFunc(level, &buffer, &entriesRead, &totalEntries, &resumeHandle);
        //qDebug("Net*Enum(level = %d) returned %d entries, total was (%d), status = %d, resume handle = %llx",
        //    level, entriesRead, totalEntries, nStatus, resumeHandle);

        // buffer must always be freed, even if Net*Enum fails
        ScopedNetApiBuffer<T> groupInfo((T*)buffer);
        if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) {
            for (DWORD i = 0; total < maxCount && i < entriesRead; i++, total++) {
                callback(groupInfo.get()[i]);
            }
        } else {
            qWarning("NetApi enumerate function failed: status = %d", nStatus);
        }
    } while (nStatus == ERROR_MORE_DATA);
}

template<class T, class Callback>
void enumerateAllUsers(uint maxCount, Callback callback) {
    netApiEnumerate<T>(maxCount, callback, [](int level, LPBYTE* buffer, DWORD* count, DWORD* total, quint64* resumeHandle) {
        // pass 0 as filter -> get all users
        // Why does this function take a DWORD* as resume handle and NetUserEnum/NetGroupGetUsers a UINT64*
        // Great API design by Microsoft...
        //casting the uint64* to uint32* is fine, it just writes to the first 32 bits
        return NetUserEnum(nullptr, level, 0, buffer, MAX_PREFERRED_LENGTH, count, total, (PDWORD)resumeHandle);
    });
}

QList<KUser> KUser::allUsers(uint maxCount)
{
    QList<KUser> result;
    // No advantage if we take a USER_INFO_11, since there is no way of copying it
    // and it is not owned by this function!
    // -> get a USER_INFO_0 instead and then use KUser(QString)
    // USER_INFO_23 or USER_INFO_23 would be ideal here since they contains a SID,
    // but that fails with error code 0x7c (bad level)
    enumerateAllUsers<USER_INFO_0>(maxCount, [&result](const USER_INFO_0& info) {
        result.append(KUser(QString::fromWCharArray(info.usri0_name)));
    });
    return result;
}

QStringList KUser::allUserNames(uint maxCount)
{
    QStringList result;
    enumerateAllUsers<USER_INFO_0>(maxCount, [&result](const USER_INFO_0& info) {
        result.append(QString::fromWCharArray(info.usri0_name));
    });
    return result;
}

template<typename T, class Callback>
void enumerateAllGroups(uint maxCount, Callback callback) {
    netApiEnumerate<T>(maxCount, callback, [](int level, LPBYTE* buffer, DWORD* count, DWORD* total, quint64* resumeHandle) {
        return NetGroupEnum(nullptr, level, buffer, MAX_PREFERRED_LENGTH, count, total, resumeHandle);
    });
}

QList<KUserGroup> KUserGroup::allGroups(uint maxCount)
{
    QList<KUserGroup> result;
    // MSDN documentation say 3 is a valid level, however the function fails with invalid level!!!
    // User GROUP_INFO_0 instead...
    enumerateAllGroups<GROUP_INFO_0>(maxCount, [&result](const GROUP_INFO_0 &groupInfo) {
        result.append(KUserGroup(QString::fromWCharArray(groupInfo.grpi0_name)));
    });
    return result;
}

QStringList KUserGroup::allGroupNames(uint maxCount)
{
    QStringList result;
    enumerateAllGroups<GROUP_INFO_0>(maxCount, [&result](const GROUP_INFO_0 &groupInfo) {
        result.append(QString::fromWCharArray(groupInfo.grpi0_name));
    });
    return result;
}

template<typename T, class Callback>
void enumerateGroupsForUser(uint maxCount, const QString& name, Callback callback) {
    LPCWSTR nameStr = (LPCWSTR)name.utf16();
    netApiEnumerate<T>(maxCount, callback, [&](int level, LPBYTE* buffer, DWORD* count, DWORD* total, quint64* resumeHandle) {
        NET_API_STATUS ret = NetUserGetGroups(nullptr, nameStr, level, buffer, MAX_PREFERRED_LENGTH, count, total);
        // if we return ERROR_MORE_DATA here it will result in an enless loop
        if (ret == ERROR_MORE_DATA) {
            qWarning() << "NetUserGetGroups for user" << name << "returned ERROR_MORE_DATA. This should not happen!";
                ret = NERR_Success;
        }
        return ret;
    });
}

QList<KUserGroup> KUser::groups(uint maxCount) const
{
    QList<KUserGroup> result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->loginName, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(KUserGroup(QString::fromWCharArray(info.grui0_name)));
    });
    return result;
}

QStringList KUser::groupNames(uint maxCount) const
{
    QStringList result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->loginName, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(QString::fromWCharArray(info.grui0_name));
    });
    return result;
}

template<typename T, class Callback>
void enumerateUsersForGroup(const QString &name, uint maxCount, Callback callback) {
    LPCWSTR nameStr = (LPCWSTR)name.utf16();
    netApiEnumerate<T>(maxCount, callback, [nameStr](int level, LPBYTE* buffer, DWORD* count, DWORD* total, quint64* resumeHandle) {
        return NetGroupGetUsers(nullptr, nameStr, level, buffer, MAX_PREFERRED_LENGTH, count, total, resumeHandle);
    });
}

QList<KUser> KUserGroup::users(uint maxCount) const
{
    QList<KUser> result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->name, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(KUser(QString::fromWCharArray(info.grui0_name)));
    });
    return result;
}

QStringList KUserGroup::userNames(uint maxCount) const
{
    QStringList result;
    if (!isValid()) {
        return result;
    }
    enumerateGroupsForUser<GROUP_USERS_INFO_0>(maxCount, d->name, [&result](const GROUP_USERS_INFO_0 &info) {
        result.append(QString::fromWCharArray(info.grui0_name));
    });
    return result;
}

static const auto invalidSidString = QStringLiteral("<invalid SID>");

static QString sidToString(void* sid) {
    if (!sid || !IsValidSid(sid)) {
        return invalidSidString;
    }
    WCHAR* sidStr; // allocated by ConvertStringSidToSidW, must be freed using LocalFree()
    if (!ConvertSidToStringSidW(sid, &sidStr)) {
        return invalidSidString;
    }
    QString ret = QString::fromWCharArray(sidStr);
    LocalFree(sidStr);
    return ret;
}

struct WindowsSIDWrapper : public QSharedData {
    char sidBuffer[SECURITY_MAX_SID_SIZE];
    /** @return a copy of @p sid or null if sid is not valid or an error occurs */
    static WindowsSIDWrapper* copySid(PSID sid)
    {
        if (!sid || !IsValidSid(sid)) {
            return nullptr;
        }
        //create a copy of sid
        WindowsSIDWrapper* copy = new WindowsSIDWrapper();
        bool success = CopySid(SECURITY_MAX_SID_SIZE, copy->sidBuffer, sid);
        if (!success) {
            QString sidString = sidToString(sid);
            qWarning("Failed to copy SID %s, error = %d", qPrintable(sidString), GetLastError());
            delete copy;
            return nullptr;
        }
        return copy;
    }
};

template<>
KUserOrGroupId<void*>::KUserOrGroupId()
{
}

template<>
KUserOrGroupId<void*>::~KUserOrGroupId()
{
}

template<>
KUserOrGroupId<void*>::KUserOrGroupId(const KUserOrGroupId<void*> &other)
    : data(other.data)
{
}

template<>
inline KUserOrGroupId<void*>& KUserOrGroupId<void*>::operator=(const KUserOrGroupId<void*>& other)
{
    data = other.data;
    return *this;
}

template<>
KUserOrGroupId<void*>::KUserOrGroupId(void* nativeId)
    : data(WindowsSIDWrapper::copySid(nativeId))
{
}

template<>
bool KUserOrGroupId<void*>::isValid() const
{
    return data;
}

template<>
void* KUserOrGroupId<void*>::nativeId() const
{
    if (!data) {
        return nullptr;
    }
    return data->sidBuffer;
}

template<>
bool KUserOrGroupId<void*>::operator==(const KUserOrGroupId<void*>& other) const
{
    if (data) {
        if (!other.data) {
            return false;
        }
        return EqualSid(data->sidBuffer, other.data->sidBuffer);
    }
    return !other.data; //only equal if other data is also invalid
}

template<>
bool KUserOrGroupId<void*>::operator!=(const KUserOrGroupId<void*> &other) const
{
    return !(*this == other);
}

template<>
QString KUserOrGroupId<void*>::toString() const
{
    return sidToString(data ? data->sidBuffer : nullptr);
}

/** T must be either KUserId or KGroupId, Callback has signature T(PSID, SID_NAME_USE) */
template<class T, class Callback>
static T sidFromName(const QString& name, Callback callback)
{
    if (name.isEmpty()) {
        // for some reason empty string will always return S-1-5-32 which is of type domain
        // we only want users or groups -> return invalid
        return T();
    }
    char buffer[SECURITY_MAX_SID_SIZE];
    DWORD sidLength = SECURITY_MAX_SID_SIZE;
    // ReferencedDomainName must be passed or LookupAccountNameW fails
    // Documentation says it is optional, however if not passed the function fails and returns the required size
    WCHAR domainBuffer[1024];
    DWORD domainBufferSize = 1024;
    SID_NAME_USE sidType;
    bool ok = LookupAccountNameW(nullptr, (LPCWSTR)name.utf16(), buffer, &sidLength, domainBuffer, &domainBufferSize, &sidType);
    if (!ok) {
        qWarning() << "Failed to lookup account" << name << "error code =" << GetLastError();
        return T();
    }
    return callback(buffer, sidType);
}

KUserId KUserId::fromName(const QString& name) {
    return sidFromName<KUserId>(name, [&](PSID sid, SID_NAME_USE sidType) {
        if (sidType != SidTypeUser && sidType != SidTypeDeletedAccount) {
            qWarning().nospace() << "Failed to lookup user name " << name
                << ": resulting SID " << sidToString(sid) << " is not a user."
                " Got SID type " << sidType << " instead.";
            return KUserId();
        }
        return KUserId(sid);
    });
}

KGroupId KGroupId::fromName(const QString& name) {
    return sidFromName<KGroupId>(name, [&](PSID sid, SID_NAME_USE sidType) {
        if (sidType != SidTypeGroup && sidType != SidTypeWellKnownGroup) {
            qWarning().nospace() << "Failed to lookup user name " << name
                << ": resulting SID " << sidToString(sid) << " is not a group."
                " Got SID type " << sidType << " instead.";
            return KGroupId();
        }
        return KGroupId(sid);
    });
}

static std::unique_ptr<char[]> queryProcessInformation(TOKEN_INFORMATION_CLASS type)
{
    HANDLE _token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &_token)) {
        qWarning("Failed to get the token for the current process: %d", GetLastError());
        return false;
    }
    ScopedHANDLE token(_token, handleCloser);
    // query required size
    DWORD requiredSize;
    if (!GetTokenInformation(token.get(), type, nullptr, 0, &requiredSize)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            qWarning("Failed to get the required size for the token information %d: %d",
                type, GetLastError());
            return nullptr;
        }
    }
    std::unique_ptr<char[]> buffer(new char[requiredSize]);
    if (!GetTokenInformation(token.get(), type, buffer.get(), requiredSize, &requiredSize)) {
        qWarning("Failed to get token information %d from current process: %d",
            type, GetLastError());
        return nullptr;
    }
    return buffer;
}

KUserId KUserId::currentUserId()
{
    std::unique_ptr<char[]> userTokenBuffer = queryProcessInformation(TokenUser);
    TOKEN_USER* userToken = (TOKEN_USER*)userTokenBuffer.get();
    return KUserId(userToken->User.Sid);
}

KGroupId KGroupId::currentGroupId()
{
    std::unique_ptr<char[]> primaryGroupBuffer = queryProcessInformation(TokenPrimaryGroup);
    TOKEN_PRIMARY_GROUP* primaryGroup = (TOKEN_PRIMARY_GROUP*)primaryGroupBuffer.get();
    return KGroupId(primaryGroup->PrimaryGroup);
}

KUserId KUserId::currentEffectiveUserId() { return currentUserId(); }

KGroupId KGroupId::currentEffectiveGroupId() { return currentGroupId(); }

KCOREADDONS_EXPORT uint qHash(const KUserId &id, uint seed)
{
    if (!id.isValid()) {
        return seed;
    }
    // we can't just hash the pointer since equal object must have the same hash -> hash contents
    char* sid = (char*)id.nativeId();
    return qHash(QByteArray::fromRawData(sid, GetLengthSid(sid)), seed);
}

KCOREADDONS_EXPORT uint qHash(const KGroupId &id, uint seed)
{
    if (!id.isValid()) {
        return seed;
    }
    // we can't just hash the pointer since equal object must have the same hash -> hash contents
    char* sid = (char*)id.nativeId();
    return qHash(QByteArray::fromRawData(sid, GetLengthSid(sid)), seed);
}
