/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Ian Zepp <icszepp@islc.net>
    SPDX-FileCopyrightText: 2006 Dominic Battre <dominic@battre.de>
    SPDX-FileCopyrightText: 2006 Martin Pool <mbp@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstringhandler.h"

#include <stdlib.h> // random()

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 67)
#include <QRegExp> // for the word ranges
#endif
#include <QRegularExpression>
#include <QVector>

//
// Capitalization routines
//
QString KStringHandler::capwords(const QString &text)
{
    if (text.isEmpty()) {
        return text;
    }

    const QString strippedText = text.trimmed();
    const QString space = QString(QLatin1Char(' '));
    const QStringList words = capwords(strippedText.split(space));

    QString result = text;
    result.replace(strippedText, words.join(space));
    return result;
}

QStringList KStringHandler::capwords(const QStringList &list)
{
    QStringList tmp = list;
    for (auto &str : tmp) {
        str[0] = str.at(0).toUpper();
    }
    return tmp;
}

QString KStringHandler::lsqueeze(const QString &str, const int maxlen)
{
    if (str.length() > maxlen) {
        const int part = maxlen - 3;
        return QLatin1String("...") + QStringView(str).right(part);
    } else {
        return str;
    }
}

QString KStringHandler::csqueeze(const QString &str, const int maxlen)
{
    if (str.length() > maxlen && maxlen > 3) {
        const int part = (maxlen - 3) / 2;
        const QStringView strView{str};
        return strView.left(part) + QLatin1String("...") + strView.right(part);
    } else {
        return str;
    }
}

QString KStringHandler::rsqueeze(const QString &str, const int maxlen)
{
    if (str.length() > maxlen) {
        const int part = maxlen - 3;
        return QStringView(str).left(part) + QLatin1String("...");
    } else {
        return str;
    }
}

QStringList KStringHandler::perlSplit(const QStringView sep, const QStringView str, int max)
{
    const bool ignoreMax = max == 0;

    const int sepLength = sep.size();

    QStringList list;
    int searchStart = 0;
    int sepIndex = str.indexOf(sep, searchStart);

    while (sepIndex != -1 && (ignoreMax || list.count() < max - 1)) {
        const auto chunk = str.mid(searchStart, sepIndex - searchStart);
        if (!chunk.isEmpty()) {
            list.append(chunk.toString());
        }

        searchStart = sepIndex + sepLength;
        sepIndex = str.indexOf(sep, searchStart);
    }

    const auto lastChunk = str.mid(searchStart, str.length() - searchStart);
    if (!lastChunk.isEmpty()) {
        list.append(lastChunk.toString());
    }

    return list;
}

QStringList KStringHandler::perlSplit(const QString &sep, const QString &s, int max)
{
    return perlSplit(QStringView(sep), QStringView(s), max);
}

QStringList KStringHandler::perlSplit(const QChar &sep, const QString &str, int max)
{
    return perlSplit(QStringView(&sep, 1), QStringView(str), max);
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 67)
QStringList KStringHandler::perlSplit(const QRegExp &sep, const QString &s, const int max)
{
    // nothing to split
    if (s.isEmpty()) {
        return QStringList();
    }

    const bool ignoreMax = 0 == max;

    QStringList l;

    int searchStart = 0;
    int tokenStart = sep.indexIn(s, searchStart);
    int len = sep.matchedLength();

    while (-1 != tokenStart && (ignoreMax || l.count() < max - 1)) {
        if (!s.midRef(searchStart, tokenStart - searchStart).isEmpty()) {
            l << s.mid(searchStart, tokenStart - searchStart);
        }

        searchStart = tokenStart + len;
        tokenStart = sep.indexIn(s, searchStart);
        len = sep.matchedLength();
    }

    if (!s.midRef(searchStart, s.length() - searchStart).isEmpty()) {
        l << s.mid(searchStart, s.length() - searchStart);
    }

    return l;
}
#endif

QStringList KStringHandler::perlSplit(const QRegularExpression &sep, const QString &str, int max)
{
    // nothing to split
    if (str.isEmpty()) {
        return QStringList();
    }

    const bool ignoreMax = max == 0;

    QStringList list;

    int start = 0;

    const QStringView strView(str);

    QRegularExpression separator(sep);
    separator.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);

    QRegularExpressionMatchIterator iter = separator.globalMatch(strView);
    QRegularExpressionMatch match;
    while (iter.hasNext() && (ignoreMax || list.count() < max - 1)) {
        match = iter.next();
        const QStringView chunk = strView.mid(start, match.capturedStart() - start);
        if (!chunk.isEmpty()) {
            list.append(chunk.toString());
        }

        start = match.capturedEnd();
    }

    // catch the remainder
    const QStringView lastChunk = strView.mid(start, strView.size() - start);
    if (!lastChunk.isEmpty()) {
        list.append(lastChunk.toString());
    }

    return list;
}

QString KStringHandler::tagUrls(const QString &text)
{
    QString richText(text);

    static const QRegularExpression urlEx(QStringLiteral(R"((www\.(?!\.)|(fish|ftp|http|https)://[\d\w./,:_~?=&;#@\-+%$()]+))"),
                                          QRegularExpression::UseUnicodePropertiesOption);
    // The reference \1 is going to be replaced by the matched url
    richText.replace(urlEx, QStringLiteral("<a href=\"\\1\">\\1</a>"));
    return richText;
}

QString KStringHandler::obscure(const QString &str)
{
    QString result;
    for (const QChar ch : str) {
        // yes, no typo. can't encode ' ' or '!' because
        // they're the unicode BOM. stupid scrambling. stupid.
        const ushort uc = ch.unicode();
        result += (uc <= 0x21) ? ch : QChar(0x1001F - uc);
    }

    return result;
}

bool KStringHandler::isUtf8(const char *buf)
{
    int i;
    int n;
    unsigned char c;
    bool gotone = false;

    if (!buf) {
        return true; // whatever, just don't crash
    }

#define F 0 /* character never appears in text */
#define T 1 /* character appears in plain ASCII text */
#define I 2 /* character appears in ISO-8859 text */
#define X 3 /* character appears in non-ISO extended ASCII (Mac, IBM PC) */
    /* clang-format off */
    static const unsigned char text_chars[256] = {
        /*                  BEL BS HT LF    FF CR    */
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F, /* 0x0X */
        /*                              ESC          */
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F, /* 0x1X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, /* 0x2X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, /* 0x3X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, /* 0x4X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, /* 0x5X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, /* 0x6X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F, /* 0x7X */
        /*            NEL                            */
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X, /* 0x8X */
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, /* 0x9X */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, /* 0xaX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, /* 0xbX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, /* 0xcX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, /* 0xdX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, /* 0xeX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I /* 0xfX */
    };
    /* clang-format on */

    /* *ulen = 0; */
    for (i = 0; (c = buf[i]); ++i) {
        if ((c & 0x80) == 0) { /* 0xxxxxxx is plain ASCII */
            /*
             * Even if the whole file is valid UTF-8 sequences,
             * still reject it if it uses weird control characters.
             */

            if (text_chars[c] != T) {
                return false;
            }

        } else if ((c & 0x40) == 0) { /* 10xxxxxx never 1st byte */
            return false;
        } else { /* 11xxxxxx begins UTF-8 */
            int following;

            if ((c & 0x20) == 0) { /* 110xxxxx */
                following = 1;
            } else if ((c & 0x10) == 0) { /* 1110xxxx */
                following = 2;
            } else if ((c & 0x08) == 0) { /* 11110xxx */
                following = 3;
            } else if ((c & 0x04) == 0) { /* 111110xx */
                following = 4;
            } else if ((c & 0x02) == 0) { /* 1111110x */
                following = 5;
            } else {
                return false;
            }

            for (n = 0; n < following; ++n) {
                i++;
                if (!(c = buf[i])) {
                    goto done;
                }

                if ((c & 0x80) == 0 || (c & 0x40)) {
                    return false;
                }
            }
            gotone = true;
        }
    }
done:
    return gotone; /* don't claim it's UTF-8 if it's all 7-bit */
}

#undef F
#undef T
#undef I
#undef X

QString KStringHandler::from8Bit(const char *str)
{
    if (!str) {
        return QString();
    }
    if (!*str) {
        static const QLatin1String emptyString("");
        return emptyString;
    }
    return KStringHandler::isUtf8(str) ? QString::fromUtf8(str) : QString::fromLocal8Bit(str);
}

QString KStringHandler::preProcessWrap(const QString &text)
{
    const QChar zwsp(0x200b);

    QString result;
    result.reserve(text.length());

    for (int i = 0; i < text.length(); i++) {
        const QChar c = text[i];
        const bool openingParens = (c == QLatin1Char('(') || c == QLatin1Char('{') || c == QLatin1Char('['));
        const bool singleQuote = (c == QLatin1Char('\''));
        const bool closingParens = (c == QLatin1Char(')') || c == QLatin1Char('}') || c == QLatin1Char(']'));
        const bool breakAfter = (closingParens || c.isPunct() || c.isSymbol());
        const bool isLastChar = i == (text.length() - 1);
        const bool isLower = c.isLower();
        const bool nextIsUpper = !isLastChar && text[i + 1].isUpper(); // false by default
        const bool nextIsSpace = isLastChar || text[i + 1].isSpace(); // true by default
        const bool prevIsSpace = (i == 0 || text[i - 1].isSpace() || result[result.length() - 1] == zwsp);

        // Provide a breaking opportunity before opening parenthesis
        if (openingParens && !prevIsSpace) {
            result += zwsp;
        }

        // Provide a word joiner before the single quote
        if (singleQuote && !prevIsSpace) {
            result += QChar(0x2060);
        }

        result += c;

        // Provide a breaking opportunity between camelCase and PascalCase sub-words
        const bool isCamelCase = isLower && nextIsUpper;

        if (isCamelCase || (breakAfter && !openingParens && !nextIsSpace && !singleQuote)) {
            result += zwsp;
        }
    }

    return result;
}

int KStringHandler::logicalLength(const QString &text)
{
    int length = 0;
    const auto chrs = text.toUcs4();
    for (const auto chr : chrs) {
        const auto script = QChar::script(chr);
        /* clang-format off */
        if (script == QChar::Script_Han
            || script == QChar::Script_Hangul
            || script == QChar::Script_Hiragana
            || script == QChar::Script_Katakana
            || script == QChar::Script_Yi
            || QChar::isHighSurrogate(chr)) { /* clang-format on */
            length += 2;
        } else {
            length += 1;
        }
    }
    return length;
}
