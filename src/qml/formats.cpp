/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "formats.h"

static void markCurrentFunctionAsTranslationBinding(const QObject *obj)
{
    if (auto engine = qmlEngine(obj); engine) {
        engine->markCurrentFunctionAsTranslationBinding();
    }
}

QString Formats::formatByteSize(double size, int precision) const
{
    markCurrentFunctionAsTranslationBinding(this);
    return m_format.formatByteSize(size, precision);
}

QString Formats::formatDuration(quint64 msecs, KFormat::DurationFormatOptions options) const
{
    markCurrentFunctionAsTranslationBinding(this);
    return m_format.formatDuration(msecs, options);
}

QString Formats::formatDecimalDuration(quint64 msecs, int decimalPlaces) const
{
    markCurrentFunctionAsTranslationBinding(this);
    return m_format.formatDecimalDuration(msecs, decimalPlaces);
}

QString Formats::formatSpelloutDuration(quint64 msecs) const
{
    markCurrentFunctionAsTranslationBinding(this);
    return m_format.formatSpelloutDuration(msecs);
}

QString Formats::formatRelativeDate(const QDate &date, QLocale::FormatType format) const
{
    markCurrentFunctionAsTranslationBinding(this);
    return m_format.formatRelativeDate(date, format);
}

QString Formats::formatRelativeDateTime(const QDateTime &dateTime, QLocale::FormatType format) const
{
    markCurrentFunctionAsTranslationBinding(this);
    return m_format.formatRelativeDateTime(dateTime, format);
}

QString Formats::formatDistance(double value, KFormat::DistanceFormatOptions options) const
{
    markCurrentFunctionAsTranslationBinding(this);
    return m_format.formatDistance(value, options);
}

#include "moc_formats.cpp"
