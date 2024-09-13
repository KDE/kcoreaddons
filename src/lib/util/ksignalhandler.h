/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KSIGNALHANDLER_H
#define KSIGNALHANDLER_H

#include <QObject>
#include <kcoreaddons_export.h>

class KSignalHandlerPrivate;

/*!
 * \class KSignalHandler
 * \inmodule KCoreAddons
 *
 * Allows getting ANSI C signals and forward them onto the Qt eventloop.
 *
 * It's a singleton as it relies on static data getting defined.
 *
 * \code
 * {
 *   KSignalHandler::self()->watchSignal(SIGTERM);
 *   connect(KSignalHandler::self(), &KSignalHandler::signalReceived,
 *           this, &SomeClass::handleSignal);
 *   job->start();
 * }
 * \endcode
 *
 * \since KCoreAddons 5.92
 */
class KCOREADDONS_EXPORT KSignalHandler : public QObject
{
    Q_OBJECT
public:
    ~KSignalHandler() override;

    /*!
     * Adds \a signal to be watched for. Once the process is notified about this signal, @m signalReceived will be emitted with the same \a signal as an
     * argument.
     *
     * \sa signalReceived
     */
    void watchSignal(int signal);

    /*!
     * Fetches an instance we can use to register our signals.
     */
    static KSignalHandler *self();

Q_SIGNALS:
    /*!
     * Notifies that \a signal is emitted.
     *
     * To catch a signal, we need to make sure it's registered using @m watchSignal.
     *
     * \sa watchSignal
     */
    void signalReceived(int signal);

private:
    KCOREADDONS_NO_EXPORT KSignalHandler();

    QScopedPointer<KSignalHandlerPrivate> d;
};

#endif
