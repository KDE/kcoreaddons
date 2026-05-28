/*
    SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SANDBOX_H
#define SANDBOX_H

#include <KFormat>
#include <QObject>
#include <QQmlEngine>

/*!
 * \qmltype Sandbox
 * \inqmlmodule org.kde.coreaddons
 *
 * \brief Utility functions for use inside application sandboxes such as Flatpak or Snap.
 *
 * \sa KSandbox
 * \since 6.28
 */
class Sandbox : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Sandbox)
    QML_SINGLETON

    /*!
     * \qmlproperty Type Sandbox::type
     * Returns the type of sandbox this application is running in.
     */
    Q_PROPERTY(Type type READ type CONSTANT)

public:
    /*!
     * The kind of sandbox.
     *
     * \value None Running natively or in an unsupported sandbox.
     * \value Flatpak Running in a Flatpak sandbox.
     * \value Snap Running in a Snap sandbox.
     */
    enum Type {
        None,
        Flatpak,
        Snap,
    };
    Q_ENUM(Type)

    Type type() const;
};

#endif
