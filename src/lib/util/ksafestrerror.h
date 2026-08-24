// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#pragma once

#include <QString>

#include "kcoreaddons_export.h"

/*!
    \inmodule KCoreAddons
    \brief Thread-safe variants of strerror that return different string types.
    \since TBD

    The POSIX strerror is not thread-safe. strerror_r has varying semantics.
    These wrappers provide a stable and thread-safe interface to the strerror functionality.

    Whenever you get an errno-like value you can feed it into these functions and get a somewhat descriptive
    string back. Do not use the output as sole log message, it is fairly generic.

    \code
    auto fd = open("somefile", O_RDONLY | O_CLOEXEC);
    if (::close(fd) == -1) {
        auto error = errno;
        qWarning() << "Failed to close file descriptor for somefile:" << KSafeStrerror::strerror(error);
    }
    \endcode

    If output would be too long it gets truncated!

    \warning This class is not available on Windows.
*/
namespace KSafeStrerror
{
/*! Thread-safe variant of strerror that returns a cstring. The memory is owned by the function and remains valid until the next call. */
[[nodiscard]] KCOREADDONS_EXPORT const char *strerror(int error);

/*! Thread-safe variant of strerror that returns a QString */
[[nodiscard]] KCOREADDONS_EXPORT QString strerrorQString(int error);
} // namespace KSafeStrerror
