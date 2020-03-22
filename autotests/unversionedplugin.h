/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Merry <alexmerry@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UNVERSIONEDPLUGIN_H
#define UNVERSIONEDPLUGIN_H

#include <QObject>

class UnversionedPlugin : public QObject
{
    Q_OBJECT

public:
    UnversionedPlugin(QObject *parent, const QVariantList &args);
};

#endif // UNVERSIONEDPLUGIN_H
