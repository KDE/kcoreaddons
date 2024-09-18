// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "kruntimeplatform.h"

QStringList KRuntimePlatform::runtimePlatform()
{
    const QString env = QString::fromLocal8Bit(getenv("PLASMA_PLATFORM"));
    return QStringList(env.split(QLatin1Char(':'), Qt::SkipEmptyParts));
}

bool KRuntimePlatform::isMobile()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(UBUNTU_TOUCH)
    return true;
#else
    // Mostly for debug purposes and for platforms which are always mobile,
    // such as Plasma Mobile
    if (qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_MOBILE")) {
        return QByteArrayList{"1", "true"}.contains(qgetenv("QT_QUICK_CONTROLS_MOBILE"));
    }

    return false;
#endif
}
