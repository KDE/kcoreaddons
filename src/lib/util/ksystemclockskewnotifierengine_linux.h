/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include "ksystemclockskewnotifierengine_p.h"

class KLinuxSystemClockSkewNotifierEngine : public KSystemClockSkewNotifierEngine
{
    Q_OBJECT

public:
    static KLinuxSystemClockSkewNotifierEngine *create(QObject *parent);
    ~KLinuxSystemClockSkewNotifierEngine() override;

private Q_SLOTS:
    void handleTimerCancelled();

private:
    KLinuxSystemClockSkewNotifierEngine(int fd, QObject *parent);

    int m_fd;
};
