/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Merry <alexmerry@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ALWAYSUNLOADPLUGIN_H
#define ALWAYSUNLOADPLUGIN_H

#include <QObject>

class AlwaysUnloadPlugin : public QObject
{
    Q_OBJECT

public:
    AlwaysUnloadPlugin(QObject *parent, const QVariantList &args);
};

#endif // ALWAYSUNLOADPLUGIN_H
