// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "kdatetimefactory.h"

KDateTimeFactory::KDateTimeFactory(QObject *parent)
    : QObject(parent)
{
}

KDateTime KDateTimeFactory::now() const
{
    return KDateTime(QDateTime::currentDateTime());
}

KDateTime KDateTimeFactory::fromDateTime(const QDateTime &dateTime) const
{
    return KDateTime(dateTime);
}

KDateTime KDateTimeFactory::fromLocaleTimeString(const QString &value, QLocale::FormatType format) const
{
    const auto time = QLocale().toTime(value, format);
    return time.isValid() ? KDateTime(QDateTime(QDate::currentDate(), time)) : KDateTime();
}

KDateTime KDateTimeFactory::invalid() const
{
    return KDateTime();
}
