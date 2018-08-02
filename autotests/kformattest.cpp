/*  This file is part of the KDE Frameworks

    Copyright (C) 2013 John Layt <jlayt@kde.org>,
    Copyright (C) 2010 Michael Leupold <lemma@confuego.org>
    Copyright (C) 2009 Michael Pyne <mpyne@kde.org>
    Copyright (C) 2008 Albert Astals Cid <aacid@kde.org>

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

#include "kformattest.h"

#include <QtTest>

#include "kformat.h"

void KFormatTest::formatByteSize()
{
    QLocale locale(QLocale::c());
    locale.setNumberOptions(QLocale::DefaultNumberOptions); // Qt >= 5.6 sets QLocale::OmitGroupSeparator for the C locale
    KFormat format(locale);

    QCOMPARE(format.formatByteSize(0), QStringLiteral("0 B"));
    QCOMPARE(format.formatByteSize(50), QStringLiteral("50 B"));
    QCOMPARE(format.formatByteSize(500), QStringLiteral("500 B"));
    QCOMPARE(format.formatByteSize(5000), QStringLiteral("4.9 KiB"));
    QCOMPARE(format.formatByteSize(50000), QStringLiteral("48.8 KiB"));
    QCOMPARE(format.formatByteSize(500000), QStringLiteral("488.3 KiB"));
    QCOMPARE(format.formatByteSize(5000000), QStringLiteral("4.8 MiB"));
    QCOMPARE(format.formatByteSize(50000000), QStringLiteral("47.7 MiB"));
    QCOMPARE(format.formatByteSize(500000000), QStringLiteral("476.8 MiB"));
#if (defined(__WORDSIZE) && (__WORDSIZE == 64)) || defined (_LP64) || defined(__LP64__) || defined(__ILP64__)
    QCOMPARE(format.formatByteSize(5000000000), QStringLiteral("4.7 GiB"));
    QCOMPARE(format.formatByteSize(50000000000), QStringLiteral("46.6 GiB"));
    QCOMPARE(format.formatByteSize(500000000000), QStringLiteral("465.7 GiB"));
    QCOMPARE(format.formatByteSize(5000000000000), QStringLiteral("4.5 TiB"));
    QCOMPARE(format.formatByteSize(50000000000000), QStringLiteral("45.5 TiB"));
    QCOMPARE(format.formatByteSize(500000000000000), QStringLiteral("454.7 TiB"));
#endif

    QCOMPARE(format.formatByteSize(1024.0, 1, KFormat::IECBinaryDialect), QStringLiteral("1.0 KiB"));
    QCOMPARE(format.formatByteSize(1023.0, 1, KFormat::IECBinaryDialect), QStringLiteral("1,023 B"));
    QCOMPARE(format.formatByteSize(1163000.0, 1, KFormat::IECBinaryDialect), QStringLiteral("1.1 MiB")); // 1.2 metric

    QCOMPARE(format.formatByteSize(-1024.0, 1, KFormat::IECBinaryDialect), QStringLiteral("-1.0 KiB"));
    QCOMPARE(format.formatByteSize(-1023.0, 1, KFormat::IECBinaryDialect), QStringLiteral("-1,023 B"));
    QCOMPARE(format.formatByteSize(-1163000.0, 1, KFormat::IECBinaryDialect), QStringLiteral("-1.1 MiB")); // 1.2 metric

    QCOMPARE(format.formatByteSize(1024.0, 1, KFormat::JEDECBinaryDialect), QStringLiteral("1.0 KB"));
    QCOMPARE(format.formatByteSize(1023.0, 1, KFormat::JEDECBinaryDialect), QStringLiteral("1,023 B"));
    QCOMPARE(format.formatByteSize(1163000.0, 1, KFormat::JEDECBinaryDialect), QStringLiteral("1.1 MB"));

    QCOMPARE(format.formatByteSize(-1024.0, 1, KFormat::JEDECBinaryDialect), QStringLiteral("-1.0 KB"));
    QCOMPARE(format.formatByteSize(-1023.0, 1, KFormat::JEDECBinaryDialect), QStringLiteral("-1,023 B"));
    QCOMPARE(format.formatByteSize(-1163000.0, 1, KFormat::JEDECBinaryDialect), QStringLiteral("-1.1 MB"));

    QCOMPARE(format.formatByteSize(1024.0, 1, KFormat::MetricBinaryDialect), QStringLiteral("1.0 kB"));
    QCOMPARE(format.formatByteSize(1023.0, 1, KFormat::MetricBinaryDialect), QStringLiteral("1.0 kB"));
    QCOMPARE(format.formatByteSize(1163000.0, 1, KFormat::MetricBinaryDialect), QStringLiteral("1.2 MB"));

    QCOMPARE(format.formatByteSize(-1024.0, 1, KFormat::MetricBinaryDialect), QStringLiteral("-1.0 kB"));
    QCOMPARE(format.formatByteSize(-1023.0, 1, KFormat::MetricBinaryDialect), QStringLiteral("-1.0 kB"));
    QCOMPARE(format.formatByteSize(-1163000.0, 1, KFormat::MetricBinaryDialect), QStringLiteral("-1.2 MB"));

    // Ensure all units are represented
    QCOMPARE(format.formatByteSize(2.0e9, 1, KFormat::MetricBinaryDialect), QStringLiteral("2.0 GB"));
    QCOMPARE(format.formatByteSize(3.2e12, 1, KFormat::MetricBinaryDialect), QStringLiteral("3.2 TB"));
    QCOMPARE(format.formatByteSize(4.1e15, 1, KFormat::MetricBinaryDialect), QStringLiteral("4.1 PB"));
    QCOMPARE(format.formatByteSize(6.7e18, 2, KFormat::MetricBinaryDialect), QStringLiteral("6.70 EB"));
    QCOMPARE(format.formatByteSize(5.6e20, 2, KFormat::MetricBinaryDialect), QStringLiteral("560.00 EB"));
    QCOMPARE(format.formatByteSize(2.3e22, 2, KFormat::MetricBinaryDialect), QStringLiteral("23.00 ZB"));
    QCOMPARE(format.formatByteSize(1.0e27, 1, KFormat::MetricBinaryDialect), QStringLiteral("1,000.0 YB"));

    // Spattering of specific units
    QCOMPARE(format.formatByteSize(823000, 3, KFormat::IECBinaryDialect, KFormat::UnitMegaByte), QStringLiteral("0.785 MiB"));
    QCOMPARE(format.formatByteSize(1234034.0, 4, KFormat::JEDECBinaryDialect, KFormat::UnitByte), QStringLiteral("1,234,034 B"));

    // Check examples from the documentation
    QCOMPARE(format.formatByteSize(1000, 1, KFormat::MetricBinaryDialect, KFormat::UnitKiloByte), QStringLiteral("1.0 kB"));
    QCOMPARE(format.formatByteSize(1000, 1, KFormat::IECBinaryDialect, KFormat::UnitKiloByte), QStringLiteral("1.0 KiB"));
    QCOMPARE(format.formatByteSize(1000, 1, KFormat::JEDECBinaryDialect, KFormat::UnitKiloByte), QStringLiteral("1.0 KB"));
}

void KFormatTest::formatValue()
{
    QLocale locale(QLocale::c());
    locale.setNumberOptions(QLocale::DefaultNumberOptions); // Qt >= 5.6 sets QLocale::OmitGroupSeparator for the C locale
    KFormat format(locale);

    // Check examples from the documentation
    QCOMPARE(format.formatValue(1000, KFormat::Unit::Byte, 1, KFormat::UnitPrefix::Kilo, KFormat::MetricBinaryDialect), QStringLiteral("1.0 kB"));
    QCOMPARE(format.formatValue(1000, KFormat::Unit::Byte, 1, KFormat::UnitPrefix::Kilo, KFormat::IECBinaryDialect), QStringLiteral("1.0 KiB"));
    QCOMPARE(format.formatValue(1000, KFormat::Unit::Byte, 1, KFormat::UnitPrefix::Kilo, KFormat::JEDECBinaryDialect), QStringLiteral("1.0 KB"));

    // Check examples from the documentation
    QCOMPARE(format.formatValue(1000, KFormat::Unit::Bit, 1, KFormat::UnitPrefix::Kilo, KFormat::MetricBinaryDialect), QStringLiteral("1.0 kbit"));
    QCOMPARE(format.formatValue(1000, QStringLiteral("bit"), 1, KFormat::UnitPrefix::Kilo), QStringLiteral("1.0 kbit"));
    QCOMPARE(format.formatValue(1000, QStringLiteral("bit/s"), 1, KFormat::UnitPrefix::Kilo), QStringLiteral("1.0 kbit/s"));

    QCOMPARE(format.formatValue(100, QStringLiteral("bit/s")), QStringLiteral("100.0 bit/s"));
    QCOMPARE(format.formatValue(1000, QStringLiteral("bit/s")), QStringLiteral("1.0 kbit/s"));
    QCOMPARE(format.formatValue(10e3, QStringLiteral("bit/s")), QStringLiteral("10.0 kbit/s"));
    QCOMPARE(format.formatValue(10e6, QStringLiteral("bit/s")), QStringLiteral("10.0 Mbit/s"));

    QCOMPARE(format.formatValue(0.010, KFormat::Unit::Meter, 1, KFormat::UnitPrefix::Milli, KFormat::MetricBinaryDialect), QStringLiteral("10.0 mm"));
    QCOMPARE(format.formatValue(10.12e-6, KFormat::Unit::Meter, 2, KFormat::UnitPrefix::Micro, KFormat::MetricBinaryDialect), QStringLiteral("10.12 µm"));
    QCOMPARE(format.formatValue(10.55e-6, KFormat::Unit::Meter, 1, KFormat::UnitPrefix::AutoAdjust, KFormat::MetricBinaryDialect), QStringLiteral("10.6 µm"));
}

enum TimeConstants {
    MSecsInDay = 86400000,
    MSecsInHour = 3600000,
    MSecsInMinute = 60000,
    MSecsInSecond = 1000
};

void KFormatTest::formatDuration()
{
    KFormat format(QLocale::c());

    quint64 singleSecond = 3 * MSecsInSecond + 700;
    quint64 doubleSecond = 33 * MSecsInSecond + 700;
    quint64 singleMinute = 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 doubleMinute = 38 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 singleHour = 5 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 doubleHour = 15 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 singleDay = 1 * MSecsInDay + 5 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 doubleDay = 10 * MSecsInDay + 5 * MSecsInHour + 8 * MSecsInMinute + 3 * MSecsInSecond + 700;
    quint64 roundingIssues = 2* MSecsInHour +  59 * MSecsInMinute + 59 * MSecsInSecond + 900;
    quint64 largeValue = 9999999999;

    // Default format
    QCOMPARE(format.formatDuration(singleSecond), QStringLiteral("0:00:04"));
    QCOMPARE(format.formatDuration(doubleSecond), QStringLiteral("0:00:34"));
    QCOMPARE(format.formatDuration(singleMinute), QStringLiteral("0:08:04"));
    QCOMPARE(format.formatDuration(doubleMinute), QStringLiteral("0:38:04"));
    QCOMPARE(format.formatDuration(singleHour), QStringLiteral("5:08:04"));
    QCOMPARE(format.formatDuration(doubleHour), QStringLiteral("15:08:04"));
    QCOMPARE(format.formatDuration(singleDay), QStringLiteral("29:08:04"));
    QCOMPARE(format.formatDuration(doubleDay), QStringLiteral("245:08:04"));
    QCOMPARE(format.formatDuration(roundingIssues), QStringLiteral("3:00:00"));
    QCOMPARE(format.formatDuration(largeValue), QStringLiteral("2777:46:40"));


    // ShowMilliseconds format
    KFormat::DurationFormatOptions options = KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0:00:03.700"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0:00:33.700"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("0:08:03.700"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("0:38:03.700"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("5:08:03.700"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("15:08:03.700"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("29:08:03.700"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("245:08:03.700"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("2:59:59.900"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("2777:46:39.999"));


    // HideSeconds format
    options = KFormat::HideSeconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0:00"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0:01"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("0:08"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("0:38"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("5:08"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("15:08"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("29:08"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("245:08"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("3:00"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("2777:47"));


    // FoldHours format
    options = KFormat::FoldHours;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0:04"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0:34"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("8:04"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("38:04"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("308:04"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("908:04"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("1748:04"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("14708:04"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("180:00"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("166666:40"));


    // FoldHours ShowMilliseconds format
    options = KFormat::FoldHours;
    options = options | KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0:03.700"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0:33.700"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("8:03.700"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("38:03.700"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("308:03.700"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("908:03.700"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("1748:03.700"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("14708:03.700"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("179:59.900"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("166666:39.999"));


    // InitialDuration format
    options = KFormat::InitialDuration;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0h00m04s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0h00m34s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("0h08m04s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("0h38m04s"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("5h08m04s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("15h08m04s"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("29h08m04s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("245h08m04s"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("3h00m00s"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("2777h46m40s"));


    // InitialDuration and ShowMilliseconds format
    options = KFormat::InitialDuration;
    options = options | KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0h00m03.700s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0h00m33.700s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("0h08m03.700s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("0h38m03.700s"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("5h08m03.700s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("15h08m03.700s"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("29h08m03.700s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("245h08m03.700s"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("2h59m59.900s"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("2777h46m39.999s"));


    // InitialDuration and HideSeconds format
    options = KFormat::InitialDuration;
    options = options | KFormat::HideSeconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0h00m"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0h01m"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("0h08m"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("0h38m"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("5h08m"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("15h08m"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("29h08m"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("245h08m"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("3h00m"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("2777h47m"));


    // InitialDuration and FoldHours format
    options = KFormat::InitialDuration;
    options = options | KFormat::FoldHours;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0m04s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0m34s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("8m04s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("38m04s"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("308m04s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("908m04s"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("1748m04s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("14708m04s"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("180m00s"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("166666m40s"));


    // InitialDuration and FoldHours and ShowMilliseconds format
    options = KFormat::InitialDuration;
    options = options | KFormat::FoldHours | KFormat::ShowMilliseconds;
    QCOMPARE(format.formatDuration(singleSecond, options), QStringLiteral("0m03.700s"));
    QCOMPARE(format.formatDuration(doubleSecond, options), QStringLiteral("0m33.700s"));
    QCOMPARE(format.formatDuration(singleMinute, options), QStringLiteral("8m03.700s"));
    QCOMPARE(format.formatDuration(doubleMinute, options), QStringLiteral("38m03.700s"));
    QCOMPARE(format.formatDuration(singleHour, options), QStringLiteral("308m03.700s"));
    QCOMPARE(format.formatDuration(doubleHour, options), QStringLiteral("908m03.700s"));
    QCOMPARE(format.formatDuration(singleDay, options), QStringLiteral("1748m03.700s"));
    QCOMPARE(format.formatDuration(doubleDay, options), QStringLiteral("14708m03.700s"));
    QCOMPARE(format.formatDuration(roundingIssues, options), QStringLiteral("179m59.900s"));
    QCOMPARE(format.formatDuration(largeValue,options), QStringLiteral("166666m39.999s"));
}

void KFormatTest::formatDecimalDuration()
{
    KFormat format(QLocale::c());

    QCOMPARE(format.formatDecimalDuration(10), QStringLiteral("10 millisecond(s)"));
    QCOMPARE(format.formatDecimalDuration(10, 3), QStringLiteral("10 millisecond(s)"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInSecond + 10), QStringLiteral("1.01 seconds"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInSecond + 1, 3), QStringLiteral("1.001 seconds"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInMinute + 10 * MSecsInSecond), QStringLiteral("1.17 minutes"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInMinute + 10 * MSecsInSecond, 3), QStringLiteral("1.167 minutes"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInHour + 10 * MSecsInMinute), QStringLiteral("1.17 hours"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInHour + 10 * MSecsInMinute, 3), QStringLiteral("1.167 hours"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInDay + 10 * MSecsInHour), QStringLiteral("1.42 days"));
    QCOMPARE(format.formatDecimalDuration(1 * MSecsInDay + 10 * MSecsInHour, 3), QStringLiteral("1.417 days"));
}

void KFormatTest::formatSpelloutDuration()
{
    KFormat format(QLocale::c());

    QCOMPARE(format.formatSpelloutDuration(1000), QStringLiteral("1 second(s)"));
    QCOMPARE(format.formatSpelloutDuration(5000), QStringLiteral("5 second(s)"));
    QCOMPARE(format.formatSpelloutDuration(60000), QStringLiteral("1 minute(s)"));
    QCOMPARE(format.formatSpelloutDuration(300000), QStringLiteral("5 minute(s)"));
    QCOMPARE(format.formatSpelloutDuration(3600000), QStringLiteral("1 hour(s)"));
    QCOMPARE(format.formatSpelloutDuration(18000000), QStringLiteral("5 hour(s)"));
    QCOMPARE(format.formatSpelloutDuration(75000), QStringLiteral("1 minute(s) and 15 second(s)"));
    // Problematic case #1 (there is a reference to this case on kformat.cpp)
    QCOMPARE(format.formatSpelloutDuration(119999), QStringLiteral("2 minute(s)"));
    // This case is strictly 2 hours, 15 minutes and 59 seconds. However, since the range is
    // pretty high between hours and seconds, formatSpelloutDuration always omits seconds when there
    // are hours in scene.
    QCOMPARE(format.formatSpelloutDuration(8159000), QStringLiteral("2 hour(s) and 15 minute(s)"));
    // This case is strictly 1 hour and 10 seconds. For the same reason, formatSpelloutDuration
    // detects that 10 seconds is just garbage compared to 1 hour, and omits it on the result.
    QCOMPARE(format.formatSpelloutDuration(3610000), QStringLiteral("1 hour(s)"));
}

void KFormatTest::formatRelativeDate()
{
    KFormat format(QLocale::c());

    QDate testDate = QDate::currentDate();
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat), QStringLiteral("Today"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat), QStringLiteral("Today"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat), QStringLiteral("Today"));

    testDate = QDate::currentDate().addDays(1);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat), QStringLiteral("Tomorrow"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat), QStringLiteral("Tomorrow"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat), QStringLiteral("Tomorrow"));

    testDate = QDate::currentDate().addDays(-1);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat), QStringLiteral("Yesterday"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat), QStringLiteral("Yesterday"));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat), QStringLiteral("Yesterday"));

    // Relative dates within a week are up to translators but there's no
    // expectation that day names are shortened -- per the API docs, the date
    // format is only used when the date is outside the relative window
    testDate = QDate::currentDate().addDays(-7);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QStringLiteral("Last %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QStringLiteral("Last %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QStringLiteral("Last %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));

    testDate = QDate::currentDate().addDays(7);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QStringLiteral("Next %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QStringLiteral("Next %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QStringLiteral("Next %1").arg(QLocale::c().dayName(testDate.dayOfWeek(), QLocale::LongFormat)));

    testDate = QDate::currentDate().addDays(-8);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QLocale::c().toString(testDate, QLocale::LongFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QLocale::c().toString(testDate, QLocale::ShortFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QLocale::c().toString(testDate, QLocale::NarrowFormat));

    testDate = QDate::currentDate().addDays(8);
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat),
             QLocale::c().toString(testDate, QLocale::LongFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::ShortFormat),
             QLocale::c().toString(testDate, QLocale::ShortFormat));
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::NarrowFormat),
             QLocale::c().toString(testDate, QLocale::NarrowFormat));

    testDate = QDate(); // invalid date
    QCOMPARE(format.formatRelativeDate(testDate, QLocale::LongFormat), QStringLiteral("Invalid date"));

    QDateTime testDateTime = QDateTime(QDate::currentDate(), QTime(3, 0, 0));
    QCOMPARE(format.formatRelativeDateTime(testDateTime, QLocale::ShortFormat), QStringLiteral("Today, 03:00:00"));

    testDateTime = QDateTime(QDate::currentDate().addDays(8), QTime(3, 0, 0));
    QCOMPARE(format.formatRelativeDateTime(testDateTime, QLocale::LongFormat),
             QLocale::c().toString(testDateTime, QLocale::LongFormat));
}

QTEST_MAIN(KFormatTest)
