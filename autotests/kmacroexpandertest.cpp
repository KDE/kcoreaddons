/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003, 2008 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2005 Thomas Braxton <brax108@cox.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QTest>
#include <kmacroexpander.h>

#include <QHash>
#include <QObject>

class KMacroExpanderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void expandMacros();
    void expandMacrosShellQuote();
    void expandMacrosShellQuoteParens();
    void expandMacrosSubClass();
};

class MyCExpander : public KCharMacroExpander
{
    QString exp;

public:
    MyCExpander()
        : KCharMacroExpander()
        , exp("expanded")
    {
    }

protected:
    bool expandMacro(QChar ch, QStringList &ret) override
    {
        if (ch == 'm') {
            ret = QStringList(exp);
            return true;
        }
        return false;
    }
};

class MyWExpander : public KWordMacroExpander
{
    QString exp;

public:
    MyWExpander()
        : KWordMacroExpander()
        , exp("expanded")
    {
    }

protected:
    bool expandMacro(const QString &str, QStringList &ret) override
    {
        if (str == QLatin1String("macro")) {
            ret = QStringList(exp);
            return true;
        }
        return false;
    }
};

void KMacroExpanderTest::expandMacros()
{
    QHash<QChar, QStringList> map;
    QStringList list;
    QString s;

    list << QString("Restaurant \"Chew It\"");
    map.insert('n', list);
    list.clear();
    list << QString("element1") << QString("'element2'");
    map.insert('l', list);

    s = "%% text %l text %n";
    QCOMPARE(KMacroExpander::expandMacros(s, map), QLatin1String("% text element1 'element2' text Restaurant \"Chew It\""));
    s = "text \"%l %n\" text";
    QCOMPARE(KMacroExpander::expandMacros(s, map), QLatin1String("text \"element1 'element2' Restaurant \"Chew It\"\" text"));

    QHash<QChar, QString> map2;
    map2.insert('a', "%n");
    map2.insert('f', "filename.txt");
    map2.insert('u', "https://www.kde.org/index.html");
    map2.insert('n', "Restaurant \"Chew It\"");
    s = "Title: %a - %f - %u - %n - %%";
    QCOMPARE(KMacroExpander::expandMacros(s, map2), QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\" - %"));

    QHash<QString, QString> smap;
    smap.insert("foo", "%n");
    smap.insert("file", "filename.txt");
    smap.insert("url", "https://www.kde.org/index.html");
    smap.insert("name", "Restaurant \"Chew It\"");

    s = "Title: %foo - %file - %url - %name - %";
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\" - %"));
    s = "%foo - %file - %url - %name";
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("%n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\""));

    s = "Title: %{foo} - %{file} - %{url} - %{name} - %";
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\" - %"));
    s = "%{foo} - %{file} - %{url} - %{name}";
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("%n - filename.txt - https://www.kde.org/index.html - Restaurant \"Chew It\""));

    s = "Title: %foo-%file-%url-%name-%";
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: %n-filename.txt-https://www.kde.org/index.html-Restaurant \"Chew It\"-%"));

    s = "Title: %{file} %{url";
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String("Title: filename.txt %{url"));

    s = " * Copyright (C) 2008 %{AUTHOR}";
    smap.clear();
    QCOMPARE(KMacroExpander::expandMacros(s, smap), QLatin1String(" * Copyright (C) 2008 %{AUTHOR}"));
}

void KMacroExpanderTest::expandMacrosShellQuote()
{
    QHash<QChar, QStringList> map;
    QStringList list;
    QString s;

    list << QString("Restaurant \"Chew It\"");
    map.insert('n', list);
    list.clear();
    list << QString("element1") << QString("'element2'") << QString("\"element3\"");
    map.insert('l', list);

#ifdef Q_OS_WIN
    s = "text %l %n text";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map),
             QLatin1String("text element1 'element2' \\^\"element3\\^\" \"Restaurant \"\\^\"\"Chew It\"\\^\" text"));

    s = "text \"%l %n\" text";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map),
             QLatin1String("text \"element1 'element2' \"\\^\"\"element3\"\\^\"\" Restaurant \"\\^\"\"Chew It\"\\^\"\"\" text"));
#else
    s = "text %l %n text";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map), QLatin1String("text element1 ''\\''element2'\\''' '\"element3\"' 'Restaurant \"Chew It\"' text"));

    s = "text \"%l %n\" text";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map), QLatin1String("text \"element1 'element2' \\\"element3\\\" Restaurant \\\"Chew It\\\"\" text"));
#endif

    QHash<QChar, QString> map2;
    map2.insert('a', "%n");
    map2.insert('f', "filename.txt");
    map2.insert('u', "https://www.kde.org/index.html");
    map2.insert('n', "Restaurant \"Chew It\"");

#ifdef Q_OS_WIN
    s = "Title: %a - %f - %u - %n - %% - %VARIABLE% foo";
    QCOMPARE(
        KMacroExpander::expandMacrosShellQuote(s, map2),
        QLatin1String(
            "Title: %PERCENT_SIGN%n - filename.txt - https://www.kde.org/index.html - \"Restaurant \"\\^\"\"Chew It\"\\^\" - %PERCENT_SIGN% - %VARIABLE% foo"));

    s = "kedit --caption %n %f";
    map2.insert('n', "Restaurant 'Chew It'");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant 'Chew It'\" filename.txt"));

    s = "kedit --caption \"%n\" %f";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant 'Chew It'\" filename.txt"));

    map2.insert('n', "Restaurant \"Chew It\"");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \"\\^\"\"Chew It\"\\^\"\"\" filename.txt"));

    map2.insert('n', "Restaurant %HOME%");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant %PERCENT_SIGN%HOME%PERCENT_SIGN%\" filename.txt"));

    s = "kedit c:\\%f";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit c:\\filename.txt"));

    s = "kedit \"c:\\%f\"";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit \"c:\\filename.txt\""));

    map2.insert('f', "\"filename.txt\"");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit \"c:\\\\\"\\^\"\"filename.txt\"\\^\"\"\""));

    map2.insert('f', "path\\");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit \"c:\\path\\\\\"\"\""));
#else
    s = "Title: %a - %f - %u - %n - %%";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2),
             QLatin1String("Title: %n - filename.txt - https://www.kde.org/index.html - 'Restaurant \"Chew It\"' - %"));

    s = "kedit --caption %n %f";
    map2.insert('n', "Restaurant 'Chew It'");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption 'Restaurant '\\''Chew It'\\''' filename.txt"));

    s = "kedit --caption \"%n\" %f";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant 'Chew It'\" filename.txt"));

    map2.insert('n', "Restaurant \"Chew It\"");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \\\"Chew It\\\"\" filename.txt"));

    map2.insert('n', "Restaurant $HOME");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \\$HOME\" filename.txt"));

    map2.insert('n', "Restaurant `echo hello`");
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"Restaurant \\`echo hello\\`\" filename.txt"));

    s = "kedit --caption \"`echo %n`\" %f";
    QCOMPARE(KMacroExpander::expandMacrosShellQuote(s, map2), QLatin1String("kedit --caption \"$( echo 'Restaurant `echo hello`')\" filename.txt"));
#endif
}

class DummyMacroExpander : public KMacroExpanderBase
{
public:
    DummyMacroExpander()
        : KMacroExpanderBase(QChar(0x4567))
    {
    }

protected:
    int expandPlainMacro(const QString &, int, QStringList &) override
    {
        return 0;
    }
    int expandEscapedMacro(const QString &, int, QStringList &) override
    {
        return 0;
    }
};

void KMacroExpanderTest::expandMacrosShellQuoteParens()
{
    QString s;

    s = "( echo \"just testing (parens)\" ) ) after";
    int pos = 0;
    DummyMacroExpander kmx;
    QVERIFY(kmx.expandMacrosShellQuote(s, pos));
    QCOMPARE(s.mid(pos), QLatin1String(") after"));
    QVERIFY(!kmx.expandMacrosShellQuote(s));
}

void KMacroExpanderTest::expandMacrosSubClass()
{
    QString s;

    MyCExpander mx1;
    s = "subst %m but not %n equ %%";
    mx1.expandMacros(s);
    QCOMPARE(s, QLatin1String("subst expanded but not %n equ %"));

    MyWExpander mx2;
    s = "subst %macro but not %not equ %%";
    mx2.expandMacros(s);
    QCOMPARE(s, QLatin1String("subst expanded but not %not equ %"));
}

QTEST_MAIN(KMacroExpanderTest)

#include "kmacroexpandertest.moc"
