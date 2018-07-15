/*  This file is part of the KDE Frameworks

    Copyright (C) 2013 Alex Merry <alex.merry@kdemail.net>
    Copyright (C) 2013 John Layt <jlayt@kde.org>
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

#include "kformatprivate_p.h"

#include <QDateTime>

#include <math.h>

KFormatPrivate::KFormatPrivate(const QLocale &locale)
{
    m_locale = locale;
}

KFormatPrivate::~KFormatPrivate()
{
}

constexpr double bpow(int exp) {
    return (exp > 0) ? 2.0 * bpow(exp - 1) :
        (exp < 0) ? 0.5 * bpow(exp + 1) :
        1.0;
}

QString KFormatPrivate::formatValue(double value,
                        KFormat::Unit unit,
                        QString unitString,
                        int precision,
                        KFormat::UnitPrefix prefix,
                        KFormat::BinaryUnitDialect dialect) const
{
    if (dialect <= KFormat::DefaultBinaryDialect || dialect > KFormat::LastBinaryDialect) {
        dialect = KFormat::IECBinaryDialect;
    }

    if (static_cast<int>(prefix) < static_cast<int>(KFormat::UnitPrefix::Yocto) ||
        static_cast<int>(prefix) > static_cast<int>(KFormat::UnitPrefix::Yotta)) {
        prefix = KFormat::UnitPrefix::AutoAdjust;
    }

    double multiplier = 1024.0;
    if (dialect == KFormat::MetricBinaryDialect) {
        multiplier = 1000.0;
    }

    int power = 0;
    if (prefix == KFormat::UnitPrefix::AutoAdjust) {
        double adjustValue = qAbs(value);
        while (adjustValue >= multiplier) {
            adjustValue /= multiplier;
            power += 1;
        }
        while (adjustValue && adjustValue < 1.0) {
            adjustValue *= multiplier;
            power -= 1;
        }
        const KFormat::UnitPrefix map[] = {
            KFormat::UnitPrefix::Yocto, // -8
            KFormat::UnitPrefix::Zepto,
            KFormat::UnitPrefix::Atto,
            KFormat::UnitPrefix::Femto,
            KFormat::UnitPrefix::Pico,
            KFormat::UnitPrefix::Nano,
            KFormat::UnitPrefix::Micro,
            KFormat::UnitPrefix::Milli,
            KFormat::UnitPrefix::Unity, // 0
            KFormat::UnitPrefix::Kilo,
            KFormat::UnitPrefix::Mega,
            KFormat::UnitPrefix::Giga,
            KFormat::UnitPrefix::Tera,
            KFormat::UnitPrefix::Peta,
            KFormat::UnitPrefix::Exa,
            KFormat::UnitPrefix::Zetta,
            KFormat::UnitPrefix::Yotta, // 8
        };
        power = std::max(-8, std::min(8, power));
        prefix = map[power + 8];
    }

    if (prefix == KFormat::UnitPrefix::Unity &&
        unit == KFormat::Unit::Byte) {
        precision = 0;
    }

    struct PrefixMapEntry
    {
        KFormat::UnitPrefix prefix;
        double decimalFactor;
        double binaryFactor;
        QChar prefixChar;
    };

    const PrefixMapEntry map[] = {
        { KFormat::UnitPrefix::Yocto, 1e-24, bpow(-80), u'y' },
        { KFormat::UnitPrefix::Zepto, 1e-21, bpow(-70), u'z' },
        { KFormat::UnitPrefix::Atto,  1e-18, bpow(-60), u'a' },
        { KFormat::UnitPrefix::Femto, 1e-15, bpow(-50), u'f' },
        { KFormat::UnitPrefix::Pico,  1e-12, bpow(-40), u'p' },
        { KFormat::UnitPrefix::Nano,  1e-9,  bpow(-30), u'n' },
        { KFormat::UnitPrefix::Micro, 1e-6,  bpow(-20), u'µ' },
        { KFormat::UnitPrefix::Milli, 1e-3,  bpow(-10), u'm' },
        { KFormat::UnitPrefix::Unity, 1.0, 1.0, u'\0' },
        { KFormat::UnitPrefix::Kilo,  1e3,   bpow(10), u'k' },
        { KFormat::UnitPrefix::Mega,  1e6,   bpow(20), u'M' },
        { KFormat::UnitPrefix::Giga,  1e9,   bpow(30), u'G' },
        { KFormat::UnitPrefix::Tera,  1e12,  bpow(40), u'T' },
        { KFormat::UnitPrefix::Peta,  1e15,  bpow(50), u'P' },
        { KFormat::UnitPrefix::Exa,   1e18,  bpow(60), u'E' },
        { KFormat::UnitPrefix::Zetta, 1e21,  bpow(70), u'Z' },
        { KFormat::UnitPrefix::Yotta, 1e24,  bpow(80), u'Y' },
    };

    auto entry = std::find_if(std::begin(map), std::end(map),
            [prefix](const PrefixMapEntry& e) { return e.prefix == prefix; });

    switch (unit) {
        case KFormat::Unit::Bit:
            unitString = QStringLiteral("bit");
            break;
        case KFormat::Unit::Byte:
            unitString = QStringLiteral("B");
            break;
        case KFormat::Unit::Meter:
            unitString = QStringLiteral("m");
            break;
        case KFormat::Unit::Hertz:
            unitString = QStringLiteral("Hz");
            break;
        case KFormat::Unit::Other:
            break;
    }

    if (prefix == KFormat::UnitPrefix::Unity) {
        QString numString = m_locale.toString(value, 'f', precision);
        //: value without prefix, format "<val> <unit>"
        return tr("%1 %2", "no Prefix").arg(numString, unitString);
    }

    QString prefixString;
    if (dialect == KFormat::MetricBinaryDialect) {
        value /= entry->decimalFactor;
        prefixString = entry->prefixChar;
    } else {
        value /= entry->binaryFactor;
        prefixString = QString(entry->prefixChar).toUpper();
        if (dialect == KFormat::IECBinaryDialect) {
            prefixString += u'i';
        }
    }

    QString numString = m_locale.toString(value, 'f', precision);

    //: value with prefix, format "<val> <prefix><unit>"
    return tr("%1 %2%3", "MetricBinaryDialect").arg(numString, prefixString, unitString);
}

QString KFormatPrivate::formatByteSize(double size, int precision,
                                       KFormat::BinaryUnitDialect dialect, KFormat::BinarySizeUnits units) const
{
    // Current KDE default is IECBinaryDialect
    if (dialect <= KFormat::DefaultBinaryDialect || dialect > KFormat::LastBinaryDialect) {
        dialect = KFormat::IECBinaryDialect;
    }

    // Current KDE default is to auto-adjust so the size falls in the range 0 to 1000/1024
    if (units < KFormat::DefaultBinaryUnits || units > KFormat::UnitLastUnit) {
        units = KFormat::DefaultBinaryUnits;
    }

    int unit = 0; // Selects what unit to use
    double multiplier = 1024.0;

    if (dialect == KFormat::MetricBinaryDialect) {
        multiplier = 1000.0;
    }

    // If a specific unit conversion is given, use it directly.  Otherwise
    // search until the result is in [0, multiplier] (or out of our range).
    if (units == KFormat::DefaultBinaryUnits) {
        while (qAbs(size) >= multiplier && unit < int(KFormat::UnitYottaByte)) {
            size /= multiplier;
            ++unit;
        }
    } else {
        // A specific unit is in use
        unit = static_cast<int>(units);
        if (unit > 0) {
            size /= pow(multiplier, unit);
        }
    }

    // Bytes, no rounding
    if (unit == 0) {
        precision = 0;
    }

    QString numString = m_locale.toString(size, 'f', precision);

    // Do not remove "//:" comments below, they are used by the translators.
    // NB: we cannot pass pluralization arguments, as the size may be negative
    if (dialect == KFormat::MetricBinaryDialect) {
        switch (unit) {
        case KFormat::UnitByte:
            //: MetricBinaryDialect size in bytes
            return tr("%1 B", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitKiloByte:
            //: MetricBinaryDialect size in 1000 bytes
            return tr("%1 kB", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitMegaByte:
            //: MetricBinaryDialect size in 10^6 bytes
            return tr("%1 MB", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitGigaByte:
            //: MetricBinaryDialect size in 10^9 bytes
            return tr("%1 GB", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitTeraByte:
            //: MetricBinaryDialect size in 10^12 bytes
            return tr("%1 TB", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitPetaByte:
            //: MetricBinaryDialect size in 10^15 bytes
            return tr("%1 PB", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitExaByte:
            //: MetricBinaryDialect size in 10^18 byte
            return tr("%1 EB", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitZettaByte:
            //: MetricBinaryDialect size in 10^21 bytes
            return tr("%1 ZB", "MetricBinaryDialect").arg(numString);
        case KFormat::UnitYottaByte:
            //: MetricBinaryDialect size in 10^24 bytes
            return tr("%1 YB", "MetricBinaryDialect").arg(numString);
        }
    } else if (dialect == KFormat::JEDECBinaryDialect) {
        switch (unit) {
        case KFormat::UnitByte:
            //: JEDECBinaryDialect memory size in bytes
            return tr("%1 B", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitKiloByte:
            //: JEDECBinaryDialect memory size in 1024 bytes
            return tr("%1 KB", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitMegaByte:
            //: JEDECBinaryDialect memory size in 10^20 bytes
            return tr("%1 MB", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitGigaByte:
            //: JEDECBinaryDialect memory size in 10^30 bytes
            return tr("%1 GB", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitTeraByte:
            //: JEDECBinaryDialect memory size in 10^40 bytes
            return tr("%1 TB", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitPetaByte:
            //: JEDECBinaryDialect memory size in 10^50 bytes
            return tr("%1 PB", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitExaByte:
            //: JEDECBinaryDialect memory size in 10^60 bytes
            return tr("%1 EB", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitZettaByte:
            //: JEDECBinaryDialect memory size in 10^70 bytes
            return tr("%1 ZB", "JEDECBinaryDialect").arg(numString);
        case KFormat::UnitYottaByte:
            //: JEDECBinaryDialect memory size in 10^80 bytes
            return tr("%1 YB", "JEDECBinaryDialect").arg(numString);
        }
    } else {  // KFormat::IECBinaryDialect, KFormat::DefaultBinaryDialect
        switch (unit) {
        case KFormat::UnitByte:
            //: IECBinaryDialect size in bytes
            return tr("%1 B", "IECBinaryDialect").arg(numString);
        case KFormat::UnitKiloByte:
            //: IECBinaryDialect size in 1024 bytes
            return tr("%1 KiB", "IECBinaryDialect").arg(numString);
        case KFormat::UnitMegaByte:
            //: IECBinaryDialect size in 10^20 bytes
            return tr("%1 MiB", "IECBinaryDialect").arg(numString);
        case KFormat::UnitGigaByte:
            //: IECBinaryDialect size in 10^30 bytes
            return tr("%1 GiB", "IECBinaryDialect").arg(numString);
        case KFormat::UnitTeraByte:
            //: IECBinaryDialect size in 10^40 bytes
            return tr("%1 TiB", "IECBinaryDialect").arg(numString);
        case KFormat::UnitPetaByte:
            //: IECBinaryDialect size in 10^50 bytes
            return tr("%1 PiB", "IECBinaryDialect").arg(numString);
        case KFormat::UnitExaByte:
            //: IECBinaryDialect size in 10^60 bytes
            return tr("%1 EiB", "IECBinaryDialect").arg(numString);
        case KFormat::UnitZettaByte:
            //: IECBinaryDialect size in 10^70 bytes
            return tr("%1 ZiB", "IECBinaryDialect").arg(numString);
        case KFormat::UnitYottaByte:
            //: IECBinaryDialect size in 10^80 bytes
            return tr("%1 YiB", "IECBinaryDialect").arg(numString);
        }
    }

    // Should never reach here
    Q_ASSERT(false);
    return numString;
}

enum TimeConstants {
    MSecsInDay = 86400000,
    MSecsInHour = 3600000,
    MSecsInMinute = 60000,
    MSecsInSecond = 1000
};

QString KFormatPrivate::formatDuration(quint64 msecs, KFormat::DurationFormatOptions options) const
{
    quint64 ms = msecs;
    if (options & KFormat::HideSeconds) {
        //round to nearest minute
        ms = qRound(ms / (qreal)MSecsInMinute) * MSecsInMinute ;
    } else if (!(options & KFormat::ShowMilliseconds)) {
        //round to nearest second
        ms = qRound(ms / (qreal)MSecsInSecond) * MSecsInSecond ;
    }

    int hours = ms / MSecsInHour;
    ms = ms % MSecsInHour;
    int minutes = ms / MSecsInMinute;
    ms = ms % MSecsInMinute;
    int seconds = ms / MSecsInSecond;
    ms = ms % MSecsInSecond;

    if ((options & KFormat::InitialDuration) == KFormat::InitialDuration) {

        if ((options & KFormat::FoldHours) == KFormat::FoldHours
                && (options & KFormat::ShowMilliseconds) == KFormat::ShowMilliseconds) {
            //: @item:intext Duration format minutes, seconds and milliseconds
            return tr("%1m%2.%3s").arg(hours * 60 + minutes, 1, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'))
                   .arg(ms, 3, 10, QLatin1Char('0'));
        } else if ((options & KFormat::FoldHours) == KFormat::FoldHours) {
            //: @item:intext Duration format minutes and seconds
            return tr("%1m%2s").arg(hours * 60 + minutes, 1, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'));
        } else if ((options & KFormat::HideSeconds) == KFormat::HideSeconds) {
            //: @item:intext Duration format hours and minutes
            return tr("%1h%2m").arg(hours, 1, 10, QLatin1Char('0'))
                   .arg(minutes, 2, 10, QLatin1Char('0'));
        } else if ((options & KFormat::ShowMilliseconds) == KFormat::ShowMilliseconds) {
            //: @item:intext Duration format hours, minutes, seconds, milliseconds
            return tr("%1h%2m%3.%4s").arg(hours, 1, 10, QLatin1Char('0'))
                   .arg(minutes, 2, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'))
                   .arg(ms, 3, 10, QLatin1Char('0'));
        } else { // Default
            //: @item:intext Duration format hours, minutes, seconds
            return tr("%1h%2m%3s").arg(hours, 1, 10, QLatin1Char('0'))
                   .arg(minutes, 2, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'));
        }

    } else {

        if ((options & KFormat::FoldHours) == KFormat::FoldHours
                && (options & KFormat::ShowMilliseconds) == KFormat::ShowMilliseconds) {
            //: @item:intext Duration format minutes, seconds and milliseconds
            return tr("%1:%2.%3").arg(hours * 60 + minutes, 1, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'))
                   .arg(ms, 3, 10, QLatin1Char('0'));
        } else if ((options & KFormat::FoldHours) == KFormat::FoldHours) {
            //: @item:intext Duration format minutes and seconds
            return tr("%1:%2").arg(hours * 60 + minutes, 1, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'));
        } else if ((options & KFormat::HideSeconds) == KFormat::HideSeconds) {
            //: @item:intext Duration format hours and minutes
            return tr("%1:%2").arg(hours, 1, 10, QLatin1Char('0'))
                   .arg(minutes, 2, 10, QLatin1Char('0'));
        } else if ((options & KFormat::ShowMilliseconds) == KFormat::ShowMilliseconds) {
            //: @item:intext Duration format hours, minutes, seconds, milliseconds
            return tr("%1:%2:%3.%4").arg(hours, 1, 10, QLatin1Char('0'))
                   .arg(minutes, 2, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'))
                   .arg(ms, 3, 10, QLatin1Char('0'));
        } else { // Default
            //: @item:intext Duration format hours, minutes, seconds
            return tr("%1:%2:%3").arg(hours, 1, 10, QLatin1Char('0'))
                   .arg(minutes, 2, 10, QLatin1Char('0'))
                   .arg(seconds, 2, 10, QLatin1Char('0'));
        }

    }

    Q_UNREACHABLE();
    return QString();
}

QString KFormatPrivate::formatDecimalDuration(quint64 msecs, int decimalPlaces) const
{
    if (msecs >= MSecsInDay) {
        //: @item:intext %1 is a real number, e.g. 1.23 days
        return tr("%1 days").arg(m_locale.toString(msecs / (MSecsInDay * 1.0), 'f', decimalPlaces));
    } else if (msecs >= MSecsInHour) {
        //: @item:intext %1 is a real number, e.g. 1.23 hours
        return tr("%1 hours").arg(m_locale.toString(msecs / (MSecsInHour * 1.0), 'f',  decimalPlaces));
    } else if (msecs >= MSecsInMinute) {
        //: @item:intext %1 is a real number, e.g. 1.23 minutes
        return tr("%1 minutes").arg(m_locale.toString(msecs / (MSecsInMinute * 1.0), 'f',  decimalPlaces));
    } else if (msecs >= MSecsInSecond) {
        //: @item:intext %1 is a real number, e.g. 1.23 seconds
        return tr("%1 seconds").arg(m_locale.toString(msecs / (MSecsInSecond * 1.0), 'f',  decimalPlaces));
    }
    //: @item:intext %1 is a whole number
    //~ singular %n millisecond
    //~ plural %n milliseconds
    return tr("%n millisecond(s)", nullptr, msecs);
}

enum DurationUnits {
    Days = 0,
    Hours,
    Minutes,
    Seconds
};

static QString formatSingleDuration(DurationUnits units, int n)
{
    // NB: n is guaranteed to be non-negative
    switch (units) {
    case Days:
        //: @item:intext %n is a whole number
        //~ singular %n day
        //~ plural %n days
        return KFormatPrivate::tr("%n day(s)", nullptr, n);
    case Hours:
        //: @item:intext %n is a whole number
        //~ singular %n hour
        //~ plural %n hours
        return KFormatPrivate::tr("%n hour(s)", nullptr, n);
    case Minutes:
        //: @item:intext %n is a whole number
        //~ singular %n minute
        //~ plural %n minutes
        return KFormatPrivate::tr("%n minute(s)", nullptr, n);
    case Seconds:
        //: @item:intext %n is a whole number
        //~ singular %n second
        //~ plural %n seconds
        return KFormatPrivate::tr("%n second(s)", nullptr, n);
    }
    Q_ASSERT(false);
    return QString();
}

QString KFormatPrivate::formatSpelloutDuration(quint64 msecs) const
{
    quint64 ms = msecs;
    int days = ms / MSecsInDay;
    ms = ms % (MSecsInDay);
    int hours = ms / MSecsInHour;
    ms = ms % MSecsInHour;
    int minutes = ms / MSecsInMinute;
    ms = ms % MSecsInMinute;
    int seconds = qRound(ms / 1000.0);

    // Handle correctly problematic case #1 (look at KFormatTest::prettyFormatDuration())
    if (seconds == 60) {
        return formatSpelloutDuration(msecs - ms + MSecsInMinute);
    }

    if (days && hours) {
        /*: @item:intext days and hours. This uses the previous item:intext messages.
            If this does not fit the grammar of your language please contact the i18n team to solve the problem */
        return tr("%1 and %2").arg(formatSingleDuration(Days, days))
               .arg(formatSingleDuration(Hours, hours));
    } else if (days) {
        return formatSingleDuration(Days, days);
    } else if (hours && minutes) {
        /*: @item:intext hours and minutes. This uses the previous item:intext messages.
            If this does not fit the grammar of your language please contact the i18n team to solve the problem */
        return tr("%1 and %2").arg(formatSingleDuration(Hours, hours))
               .arg(formatSingleDuration(Minutes, minutes));
    } else if (hours) {
        return formatSingleDuration(Hours, hours);
    } else if (minutes && seconds) {
        /*: @item:intext minutes and seconds. This uses the previous item:intext messages.
            If this does not fit the grammar of your language please contact the i18n team to solve the problem */
        return tr("%1 and %2").arg(formatSingleDuration(Minutes, minutes))
               .arg(formatSingleDuration(Seconds, seconds));
    } else if (minutes) {
        return formatSingleDuration(Minutes, minutes);
    } else {
        return formatSingleDuration(Seconds, seconds);
    }
}

QString KFormatPrivate::formatRelativeDate(const QDate &date, QLocale::FormatType format) const
{
    if (!date.isValid()) {
        return tr("Invalid date", "used when a relative date string can't be generated because the date is invalid");
    }

    const qint64 daysTo = QDate::currentDate().daysTo(date);
    if (daysTo > 7 || daysTo < -7) {
        return m_locale.toString(date, format);
    }

    switch (daysTo) {
    case 1:
        return tr("Tomorrow");
    case 0:
        return tr("Today");
    case -1:
        return tr("Yesterday");
    }

    if (daysTo < -1) {
        switch (date.dayOfWeek()) {
        case 1:
            return tr("Last Monday",    "most recent such day before today");
        case 2:
            return tr("Last Tuesday",   "most recent such day before today");
        case 3:
            return tr("Last Wednesday", "most recent such day before today");
        case 4:
            return tr("Last Thursday",  "most recent such day before today");
        case 5:
            return tr("Last Friday",    "most recent such day before today");
        case 6:
            return tr("Last Saturday",  "most recent such day before today");
        case 7:
            return tr("Last Sunday",    "most recent such day before today");
        }
    } else if (daysTo > 1) {
        switch (date.dayOfWeek()) {
        case 1:
            return tr("Next Monday",    "the next such day after today");
        case 2:
            return tr("Next Tuesday",   "the next such day after today");
        case 3:
            return tr("Next Wednesday", "the next such day after today");
        case 4:
            return tr("Next Thursday",  "the next such day after today");
        case 5:
            return tr("Next Friday",    "the next such day after today");
        case 6:
            return tr("Next Saturday",  "the next such day after today");
        case 7:
            return tr("Next Sunday",    "the next such day after today");
        }
    }
    Q_UNREACHABLE();
}

QString KFormatPrivate::formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const
{
    const qint64 daysTo = QDate::currentDate().daysTo(dateTime.date());
    if (daysTo > 7 || daysTo < -7) {
        return m_locale.toString(dateTime, format);
    }

    /*: relative datetime with %1 result of formatReleativeDate() and %2 the formatted time
        If this does not fit the grammar of your language please contact the i18n team to solve the problem */
    return tr("%1, %2").arg(formatRelativeDate(dateTime.date(), format))
           .arg(m_locale.toString(dateTime.time(), format));
}
