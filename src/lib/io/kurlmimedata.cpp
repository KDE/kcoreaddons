/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2005-2012 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kurlmimedata.h"
#include <QMimeData>
#include <QStringList>

static QString kdeUriListMime()
{
    return QStringLiteral("application/x-kde4-urilist");
} // keep this name "kde4" for compat.

static QByteArray uriListData(const QList<QUrl> &urls)
{
    // compatible with qmimedata.cpp encoding of QUrls
    QByteArray result;
    for (int i = 0; i < urls.size(); ++i) {
        result += urls.at(i).toEncoded();
        result += "\r\n";
    }
    return result;
}

void KUrlMimeData::setUrls(const QList<QUrl> &urls, const QList<QUrl> &mostLocalUrls, QMimeData *mimeData)
{
    // Export the most local urls as text/uri-list and plain text, for non KDE apps.
    mimeData->setUrls(mostLocalUrls); // set text/uri-list and text/plain

    // Export the real KIO urls as a kde-specific mimetype
    mimeData->setData(kdeUriListMime(), uriListData(urls));
}

void KUrlMimeData::setMetaData(const MetaDataMap &metaData, QMimeData *mimeData)
{
    QByteArray metaDataData; // :)
    for (auto it = metaData.cbegin(); it != metaData.cend(); ++it) {
        metaDataData += it.key().toUtf8();
        metaDataData += "$@@$";
        metaDataData += it.value().toUtf8();
        metaDataData += "$@@$";
    }
    mimeData->setData(QStringLiteral("application/x-kio-metadata"), metaDataData);
}

QStringList KUrlMimeData::mimeDataTypes()
{
    return QStringList{kdeUriListMime(), QStringLiteral("text/uri-list")};
}

static QList<QUrl> extractKdeUriList(const QMimeData *mimeData)
{
    QList<QUrl> uris;
    const QByteArray ba = mimeData->data(kdeUriListMime());
    // Code from qmimedata.cpp
    QList<QByteArray> urls = ba.split('\n');
    uris.reserve(urls.size());
    for (int i = 0; i < urls.size(); ++i) {
        QByteArray data = urls.at(i).trimmed();
        if (!data.isEmpty()) {
            uris.append(QUrl::fromEncoded(data));
        }
    }
    return uris;
}

QList<QUrl> KUrlMimeData::urlsFromMimeData(const QMimeData *mimeData, DecodeOptions decodeOptions, MetaDataMap *metaData)
{
    QList<QUrl> uris;
    if (decodeOptions == PreferLocalUrls) {
        // Extracting uris from text/uri-list, use the much faster QMimeData method urls()
        uris = mimeData->urls();
        if (uris.isEmpty()) {
            uris = extractKdeUriList(mimeData);
        }
    } else {
        uris = extractKdeUriList(mimeData);
        if (uris.isEmpty()) {
            uris = mimeData->urls();
        }
    }

    if (metaData) {
        const QByteArray metaDataPayload = mimeData->data(QStringLiteral("application/x-kio-metadata"));
        if (!metaDataPayload.isEmpty()) {
            QString str = QString::fromUtf8(metaDataPayload.constData());
            Q_ASSERT(str.endsWith(QLatin1String("$@@$")));
            str.chop(4);
            const QStringList lst = str.split(QStringLiteral("$@@$"));
            bool readingKey = true; // true, then false, then true, etc.
            QString key;
            for (const QString &s : lst) {
                if (readingKey) {
                    key = s;
                } else {
                    metaData->insert(key, s);
                }
                readingKey = !readingKey;
            }
            Q_ASSERT(readingKey); // an odd number of items would be, well, odd ;-)
        }
    }
    return uris;
}
