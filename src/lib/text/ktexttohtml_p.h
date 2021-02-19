/*
    SPDX-FileCopyrightText: 2002 Dave Corrie <kde@davecorrie.com>
    SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTTOHTML_P_H
#define KTEXTTOHTML_P_H

#include "kcoreaddons_export.h"
#include "ktexttohtmlemoticonsinterface.h"

class KTextToHTMLEmoticonsDummy : public KTextToHTMLEmoticonsInterface
{
public:
    QString parseEmoticons(const QString &text, bool strictParse = false, const QStringList &exclude = QStringList()) override
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
    QString highlightedText();

    QString mText;
    int mMaxUrlLen;
    int mMaxAddressLen;
    int mPos;
};

#endif
