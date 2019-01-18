/*
  Copyright (c) 2002 Dave Corrie <kde@davecorrie.com>
  Copyright (c) 2014 Daniel Vrátil <dvratil@redhat.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KTEXTTOHTML_P_H
#define KTEXTTOHTML_P_H

#include <QObject>

#include "kcoreaddons_export.h"
#include "ktexttohtmlemoticonsinterface.h"

class KTextToHTMLEmoticonsDummy : public KTextToHTMLEmoticonsInterface
{
public:
    QString parseEmoticons(const QString &text,
                           bool strictParse = false,
                           const QStringList &exclude = QStringList()) override
    {
        Q_UNUSED(strictParse);
        Q_UNUSED(exclude);
        return text;
    }
};

class KTextToHTMLHelper
{
public:
    KTextToHTMLHelper(const QString &plainText, int pos = 0, int maxUrlLen = 4096, int maxAddressLen = 255);

    KTextToHTMLEmoticonsInterface *emoticonsInterface() const;

    QString getEmailAddress();
    QString getPhoneNumber();
    bool atUrl() const;
    bool isEmptyUrl(const QString &url) const;
    QString getUrl(bool *badurl = nullptr);
    QString pngToDataUrl(const QString &pngPath) const;
    QString highlightedText();

    QString mText;
    int mMaxUrlLen;
    int mMaxAddressLen;
    int mPos;
};

#endif
