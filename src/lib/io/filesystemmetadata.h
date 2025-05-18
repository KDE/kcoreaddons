/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FILESYSTEMMETADMETADATA_H
#define FILESYSTEMMETADMETADATA_H

#include "kcoreaddons_export.h"
#include <QStringList>

class FilesystemMetaDataPrivate;

/*!
 * \class FilesystemMetaData
 * \inheaderfile FilesystemMetaData
 *
 * \brief The FilesystemMetaData class can be used to read and set file-system meta data of files: extended attributes in Unix, ADS data for Windows.
 *
 * Use FilesystemMetaData::forFile to constract
 */
class KCOREADDONS_EXPORT FilesystemMetaData
{
public:
    FilesystemMetaData(const FilesystemMetaData &rhs);
    virtual ~FilesystemMetaData();

    /*!
     * \value NoError i.e. Success
     * \value UnknownError An error that's not currently handled specifically
     * \value NotSupported Underlying filesystem does not provide extended attributes features
     * \value NoSpace There is insufficient space remaining to store the extended attribute
     * \value MissingPermission Process doesn't have write permission to the file or the file is marked append-only
     * \value ValueTooBig The value size exceeds the maximum size allowed per-value (64 kB for Linux VFS
     * \value NameToolong The attribute name is too long (255 bytes for Linux VFS)
     *
     */
    enum Error {
        NoError = 0,
        UnknownError,
        NotSupported,
        NoSpace,
        MissingPermission,
        ValueTooBig,
        NameToolong,
    };

    /*!
     * Construct a FilesystemMetaData to access metadata for file.
     *
     * \a filePath the path of the file
     *
     * It return std::nullopt_t if the file sytem metadata is not supported for the file
     */
    static std::optional<FilesystemMetaData> forFile(const QString &filePath);

    const FilesystemMetaData &operator=(const FilesystemMetaData &rhs);

    /*!
     * The file path corresponding to this FilesystemMetaData
     */
    QString filePath() const;

    /*!
     * Whether or not the file system supports file metadata
     */
    bool isSupported() const;

    /*!
     * Return the value of specified metadata attribute.
     *
     * \a name the name of the metadata attribute
     */
    QString attribute(QStringView name) const;

    /*!
     * Adds a metadata attribute to the file.
     *
     * \a name the name of the metadata to add
     * \a value the value to set
     */
    Error setAttribute(const QString &name, const QString &value);

    /*!
     * Whether the file has a value for the specified attribute
     *
     * \a name the name of the metadata attribute
     */
    bool hasAttribute(QStringView name) const;

    /*!
     * Returns the list of the names of metadata attributes
     */
    QStringList attributes() const;

private:
    explicit FilesystemMetaData(const QString &filePath);

    const std::unique_ptr<FilesystemMetaDataPrivate> d;
};

#endif // FILESYSTEMMETADMETADATA_H
