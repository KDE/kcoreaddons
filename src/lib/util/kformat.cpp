/*  This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2013 Alex Merry <alex.merry@kdemail.net>
    SPDX-FileCopyrightText: 2013 John Layt <jlayt@kde.org>
    SPDX-FileCopyrightText: 2010 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2009 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2008 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kformatprivate_p.h"
#include <sys/stat.h>

KFormat::KFormat(const QLocale &locale)
    : d(new KFormatPrivate(locale))
{
}

KFormat::KFormat(const KFormat &other)
    : d(other.d)
{
}

KFormat &KFormat::operator=(const KFormat &other)
{
    d = other.d;
    return *this;
}

KFormat::~KFormat()
{
}

QString KFormat::formatByteSize(double size, int precision, KFormat::BinaryUnitDialect dialect, KFormat::BinarySizeUnits units) const
{
    return d->formatByteSize(size, precision, dialect, units);
}

QString KFormat::formatValue(double value, KFormat::Unit unit, int precision, KFormat::UnitPrefix prefix, KFormat::BinaryUnitDialect dialect) const
{
    return d->formatValue(value, unit, QString(), precision, prefix, dialect);
}

QString KFormat::formatValue(double value, const QString &unit, int precision, KFormat::UnitPrefix prefix) const
{
    return d->formatValue(value, KFormat::Unit::Other, unit, precision, prefix, MetricBinaryDialect);
}

// TODO KF6 Merge both methods
QString KFormat::formatValue(double value, const QString &unit, int precision, KFormat::UnitPrefix prefix, KFormat::BinaryUnitDialect dialect) const
{
    return d->formatValue(value, KFormat::Unit::Other, unit, precision, prefix, dialect);
}

QString KFormat::formatDuration(quint64 msecs, KFormat::DurationFormatOptions options) const
{
    return d->formatDuration(msecs, options);
}

QString KFormat::formatDecimalDuration(quint64 msecs, int decimalPlaces) const
{
    return d->formatDecimalDuration(msecs, decimalPlaces);
}

QString KFormat::formatSpelloutDuration(quint64 msecs) const
{
    return d->formatSpelloutDuration(msecs);
}

QString KFormat::formatRelativeDate(const QDate &date, QLocale::FormatType format) const
{
    return d->formatRelativeDate(date, format);
}

QString KFormat::formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const
{
    return d->formatRelativeDateTime(dateTime, format);
}

QString KFormat::formatPermission(mode_t perm, mode_t st_mode, bool hasAcl) const
{
    static char buffer[12];

    // Include the type in the first char like ls does; people are more used to seeing it,
    // even though it's not really part of the permissions per se.
    switch(st_mode & S_IFMT) {
    case S_IFLNK:
        buffer[0] = 'l';
        break;
    case S_IFDIR:
        buffer[0] = 'd';
        break;
#ifdef Q_OS_UNIX
    case S_IFSOCK:
        buffer[0] = 's';
        break;
    case S_IFCHR:
        buffer[0] = 'c';
        break;
    case S_IFBLK:
        buffer[0] = 'b';
        break;
    case S_IFIFO:
        buffer[0] = 'p';
        break;
#endif // Q_OS_UNIX
    default:
        // unknown and regular
        buffer[0] = '-';
    }

    buffer[1] = (((perm & S_IRUSR) == S_IRUSR) ? 'r' : '-');
    buffer[2] = (((perm & S_IWUSR) == S_IWUSR) ? 'w' : '-');

    if ((perm & (S_IXUSR | S_ISUID)) == (S_IXUSR | S_ISUID)) {
        buffer[3] = 's';
    } else if ((perm & (S_IXUSR | S_ISUID)) == S_ISUID) {
        buffer[3] = 'S';
    } else if ((perm & (S_IXUSR | S_ISUID)) == S_IXUSR) {
        buffer[3] = 'x';
    } else {
        buffer[3] = '-';
    }

    buffer[4] = (((perm & S_IRGRP) == S_IRGRP) ? 'r' : '-');
    buffer[5] = (((perm & S_IWGRP) == S_IWGRP) ? 'w' : '-');

    if ((perm & (S_IXGRP | S_ISGID)) == (S_IXGRP | S_ISGID)) {
        buffer[6] = 's';
    } else if ((perm & (S_IXGRP | S_ISGID)) == S_ISGID) {
        buffer[6] = 'S';
    } else if ((perm & (S_IXGRP | S_ISGID)) == S_IXGRP) {
        buffer[6] = 'x';
    } else {
        buffer[6] = '-';
    }

    buffer[7] = (((perm & S_IROTH) == S_IROTH) ? 'r' : '-');
    buffer[8] = (((perm & S_IWOTH) == S_IWOTH) ? 'w' : '-');

    if ((perm & (S_IXOTH | S_ISVTX)) == (S_IXOTH | S_ISVTX)) {
        buffer[9] = 't';
    } else if ((perm & (S_IXOTH | S_ISVTX)) == S_ISVTX) {
        buffer[9] = 'T';
    } else if ((perm & (S_IXOTH | S_ISVTX)) == S_IXOTH) {
        buffer[9] = 'x';
    } else {
        buffer[9] = '-';
    }

    if (hasAcl) {
        buffer[10] = '+';
        buffer[11] = 0;
    } else {
        buffer[10] = 0;
    }

    return QString::fromLatin1(buffer);
}

#include "moc_kformat.cpp"
