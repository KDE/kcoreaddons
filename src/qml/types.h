// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef TYPES_H
#define TYPES_H

#include <kaboutdata.h>
#include <kjob.h>
#include <ktexttohtml.h>

#include <qqmlregistration.h>

namespace KTextToHtmlForeign
{
Q_NAMESPACE
QML_NAMED_ELEMENT(KTextToHTMLOptions)
QML_FOREIGN_NAMESPACE(KTextToHTML)
}

struct KAboutReleaseForeign {
    Q_GADGET
    QML_VALUE_TYPE(aboutRelease)
    QML_FOREIGN(KAboutRelease)
};

namespace KAboutDataForeign
{
Q_NAMESPACE
QML_NAMED_ELEMENT(AboutUrlType)
QML_FOREIGN_NAMESPACE(KAboutData)
}

class KJobForeign : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KJob)
    QML_FOREIGN(KJob)
    QML_UNCREATABLE("KJob cannot be created from QML")
};
#endif
