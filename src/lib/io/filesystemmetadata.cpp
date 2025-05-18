/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filesystemmetadata.h"

#if defined(Q_OS_WIN)
#include "xattr_p_win.h"
#else
#include "xattr_p_unix.h"
#endif

class FilesystemMetaDataPrivate
{
public:
    QString filePath;
};

std::optional<FilesystemMetaData> FilesystemMetaData::forFile(const QString &filePath)
{
    FilesystemMetaData md(filePath);
    if (!md.isSupported()) {
        return {};
    }
    return md;
}

FilesystemMetaData::FilesystemMetaData(const QString &filePath)
    : d(new FilesystemMetaDataPrivate)
{
    d->filePath = filePath;
}

FilesystemMetaData::FilesystemMetaData(const FilesystemMetaData &rhs)
    : d(new FilesystemMetaDataPrivate(*rhs.d))
{
}

FilesystemMetaData::~FilesystemMetaData() = default;

const FilesystemMetaData &FilesystemMetaData::operator=(const FilesystemMetaData &rhs)
{
    d->filePath = rhs.d->filePath;
    return *this;
}

QString FilesystemMetaData::filePath() const
{
    return d->filePath;
}

FilesystemMetaData::Error FilesystemMetaData::setAttribute(const QString &key, const QString &value)
{
    int result;
    if (!value.isEmpty()) {
        result = k_setxattr(d->filePath, key, value);
    } else {
        result = k_removexattr(d->filePath, key);
    }

    if (result != 0) {
        switch (result) {
#ifdef Q_OS_UNIX
        case EDQUOT:
#endif
        case ENOSPC:
            return NoSpace;
        case ENOTSUP:
            return NotSupported;
        case EACCES:
        case EPERM:
            return MissingPermission;
#ifdef Q_OS_WIN
        case ERROR_FILENAME_EXCED_RANGE:
#endif
        case ENAMETOOLONG:
        case ERANGE:
            return NameToolong;
        case E2BIG:
            return ValueTooBig;
        default:
            return UnknownError;
        }
    }
    return NoError;
}

bool FilesystemMetaData::hasAttribute(QStringView key) const
{
    return k_hasAttribute(d->filePath, key);
}

QString FilesystemMetaData::attribute(QStringView key) const
{
    QString value;
    k_getxattr(d->filePath, key, &value);

    return value;
}

bool FilesystemMetaData::isSupported() const
{
    return k_isSupported(d->filePath);
}

QStringList FilesystemMetaData::attributes() const
{
    return k_queryAttributes(d->filePath);
}
