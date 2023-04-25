/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "formats.h"

Formats::Formats(QObject *parent)
    : QObject(parent)
    , m_format()
{
}

QString Formats::formatByteSize(double size, int precision) const
{
    return m_format.formatByteSize(size, precision);
}

QString Formats::formatDuration(quint64 msecs, KFormat::DurationFormatOptions options) const
{
    return m_format.formatDuration(msecs, options);
}

QString Formats::formatDuration(quint64 msecs, int options) const
{
    return m_format.formatDuration(msecs, static_cast<KFormat::DurationFormatOptions>(options));
}

QString Formats::formatDecimalDuration(quint64 msecs, int decimalPlaces) const
{
    return m_format.formatDecimalDuration(msecs, decimalPlaces);
}

QString Formats::formatSpelloutDuration(quint64 msecs) const
{
    return m_format.formatSpelloutDuration(msecs);
}

QString Formats::formatRelativeDate(const QDate &date, QLocale::FormatType format) const
{
    return m_format.formatRelativeDate(date, format);
}

QString Formats::formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const
{
    return m_format.formatRelativeDateTime(dateTime, format);
}
