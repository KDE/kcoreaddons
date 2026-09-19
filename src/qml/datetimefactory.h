// SPDX-FileCopyrightText: 2026 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <kdatetime.h>

#include <QDateTime>
#include <QObject>

/*!
 * \qmltype KDateTimeFactory
 * \inqmlmodule org.kde.coreaddons
 *
 * \brief Creates DateTime values from QML.
 *
 * KDateTime is a value type and cannot be instantiated directly from QML, so
 * this singleton provides the ways QML code needs to obtain one instead of
 * falling back to JavaScript's \c Date.
 */
class KDateTimeFactory : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit KDateTimeFactory(QObject *parent = nullptr);

    /*!
     * \brief Returns the current date and time.
     */
    Q_INVOKABLE KDateTime now() const;

    /*!
     * \brief Returns dateTime wrapped in a DateTime.
     */
    Q_INVOKABLE KDateTime fromDateTime(const QDateTime &dateTime) const;

    /*!
     * \brief Returns an invalid DateTime, for example to clear a bound.
     */
    Q_INVOKABLE KDateTime invalid() const;
};
}
