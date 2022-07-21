/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kmessage.h"

#include <iostream>

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 97)

class StaticMessageHandler
{
public:
    StaticMessageHandler()
    {
    }
    ~StaticMessageHandler()
    {
        delete m_handler;
    }
    StaticMessageHandler(const StaticMessageHandler &) = delete;
    StaticMessageHandler &operator=(const StaticMessageHandler &) = delete;

    /* Sets the new message handler and deletes the old one */
    void setHandler(KMessageHandler *handler)
    {
        delete m_handler;
        m_handler = handler;
    }
    KMessageHandler *handler() const
    {
        return m_handler;
    }

protected:
    KMessageHandler *m_handler = nullptr;
};
Q_GLOBAL_STATIC(StaticMessageHandler, s_messageHandler)

static void internalMessageFallback(KMessage::MessageType messageType, const QString &text, const QString &title)
{
    QString prefix;
    switch (messageType) {
    case KMessage::Error:
        prefix = QStringLiteral("ERROR: ");
        break;
    case KMessage::Fatal:
        prefix = QStringLiteral("FATAL: ");
        break;
    case KMessage::Information:
        prefix = QStringLiteral("INFORMATION: ");
        break;
    case KMessage::Sorry:
        prefix = QStringLiteral("SORRY: ");
        break;
    case KMessage::Warning:
        prefix = QStringLiteral("WARNING: ");
        break;
    }

    QString message;

    if (!title.isEmpty()) {
        message += QLatin1Char('(') + title + QLatin1Char(')');
    }

    message += prefix + text;

    // Show a message to the developer to setup a KMessageHandler
    std::cerr << "WARNING: Please setup an KMessageHandler with KMessage::setMessageHandler to display message propertly." << std::endl;
    // Show message to stdout
    std::cerr << qPrintable(message) << std::endl;
}

void KMessage::setMessageHandler(KMessageHandler *handler)
{
    // Delete old message handler.
    s_messageHandler()->setHandler(handler);
}

void KMessage::message(KMessage::MessageType messageType, const QString &text, const QString &title)
{
    // Use current message handler if available, else use stdout
    if (s_messageHandler()->handler()) {
        s_messageHandler()->handler()->message(messageType, text, title);
    } else {
        internalMessageFallback(messageType, text, title);
    }
}

#endif
