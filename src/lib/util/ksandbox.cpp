// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#include "ksandbox.h"

#include <QFileInfo>

bool KSandbox::isInside()
{
    static const bool isInside = isFlatpak() || isSnap();
    return isInside;
}

bool KSandbox::isFlatpak()
{
    static const bool isFlatpak = QFileInfo::exists(QStringLiteral("/.flatpak-info"));
    return isFlatpak;
}

bool KSandbox::isSnap()
{
    static const bool isSnap = qEnvironmentVariableIsSet("SNAP");
    return isSnap;
}
