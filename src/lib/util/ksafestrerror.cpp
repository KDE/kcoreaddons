// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2023-2026 Harald Sitter <sitter@kde.org>

#include "ksafestrerror.h"

#include <algorithm>
#include <array>

#include "kcoreaddons_debug.h"

namespace
{
constexpr auto s_maxBufferSize = 1024;
using BufferArray = std::array<char, s_maxBufferSize>;

// There are two general versions for strerror_r but the specific conditions in which they appear are a bit awkward
// to express reliably so we do some function overloading here.
// Our wrapper functions take the actual function pointer as argument through which we deduce which version we are using.
// The unused version will be discarded as unused.
// Should another version appear in the future we'll match neither and get a compile error.
using GNUSignature = char *(*)(int, char *, size_t);
using XSISignature = int (*)(int, char *, size_t);

[[maybe_unused]] inline void k_strerror_r(XSISignature strerror_r, int error, BufferArray &buffer)
{
    // The XSI version is fun because it is very loosely specified. Defensively program!
    errno = 0;
    int ret = strerror_r(error, buffer.data(), buffer.size());
    if (ret == 0) { // On success we always get 0
        // But it is unclear if it is null terminated. Make sure we at least don't run past the known end.
        buffer.back() = '\0';
        return;
    }
    // > Otherwise, an error number shall be returned to indicate the error.
    // Is it positive? Is it negative? Is it errno? Who knows?! Print and move on.
    qCWarning(KCOREADDONS_DEBUG) << "strerror_r failed with error" << ret << "while errno was" << errno << ", for incoming error" << error;
}

[[maybe_unused]] inline void k_strerror_r(GNUSignature strerror_r, int error, BufferArray &buffer)
{
    // The GNU version is fun because it returns a pointer to a static string, but only sometimes.
    // Check accordingly.
    char *ret = strerror_r(error, buffer.data(), buffer.size());
    if (ret != buffer.data()) {
        // We got a static string back. Copy it into our buffer.
        auto retSpan = std::span{ret, strnlen(ret, buffer.size()) + 1}; // +1 for null terminator.
        std::ranges::copy(retSpan.begin(), retSpan.end(), buffer.begin());
    }
    // The ret is always null terminated, by extension so is buffer now.
}
} // namespace

const char *KSafeStrerror::strerror(int error)
{
    thread_local BufferArray buffer;
    buffer.front() = '\0'; // Make extra sure we don't return garbage.
    k_strerror_r(&strerror_r, error, buffer);
    return buffer.data();
}

QString KSafeStrerror::strerrorQString(int error)
{
    return QString::fromUtf8(strerror(error));
}
