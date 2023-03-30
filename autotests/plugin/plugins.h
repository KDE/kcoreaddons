// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later
#pragma once

#include <QObject>

class Q_DECL_EXPORT MyPlugin : public QObject
{
    Q_OBJECT
public:
    MyPlugin(QObject *parent);
    ~MyPlugin() override;
};

class Q_DECL_EXPORT MyPlugin2 : public QObject
{
    Q_OBJECT
public:
    MyPlugin2(QObject *parent);
    ~MyPlugin2() override;
};
