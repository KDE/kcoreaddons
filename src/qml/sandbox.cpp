/*
    SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "sandbox.h"

#include <KSandbox>

Sandbox::Type Sandbox::type() const
{
    if (KSandbox::isFlatpak()) {
        return Flatpak;
    }

    if (KSandbox::isSnap()) {
        return Snap;
    }

    return None;
}

#include "moc_sandbox.cpp"
