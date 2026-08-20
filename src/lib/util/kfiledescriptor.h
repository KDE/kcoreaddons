// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <utility>

#include "kcoreaddons_export.h"

/*!
    \inmodule KCoreAddons
    \brief Owning container for a file descriptor that automatically closes it on destruction.
    \since TBD

    \warning This class is not supported on Windows.
 */
class KCOREADDONS_EXPORT KFileDescriptor
{
public:
    /*! Constructs an invalid KFileDescriptor */
    KFileDescriptor() = default;

    /*! Constructs a KFileDescriptor that takes ownership of the given file descriptor */
    explicit KFileDescriptor(int fd)
        : m_fd(fd)
    {
    }

    /*! Moves a KFileDescriptor */
    KFileDescriptor(KFileDescriptor &&other) noexcept
        : m_fd(std::exchange(other.m_fd, -1))
    {
    }

    /*! Moves a KFileDescriptor */
    KFileDescriptor &operator=(KFileDescriptor &&other) noexcept
    {
        reset();
        m_fd = std::exchange(other.m_fd, -1);
        return *this;
    }

    /*! Destroys the KFileDescriptor and closes the file descriptor if it is valid */
    ~KFileDescriptor()
    {
        reset();
    }

    Q_DISABLE_COPY(KFileDescriptor)

    /*! Returns true if the KFileDescriptor is valid (i.e. != -1) */
    explicit operator bool() const
    {
        return isValid();
    }

    /*! Returns true if the KFileDescriptor is valid (i.e. != -1) */
    [[nodiscard]] bool isValid() const
    {
        return m_fd != -1;
    }

    /*! Returns the file descriptor. The KFileDescriptor continues to have ownership! */
    [[nodiscard]] int get() const
    {
        return m_fd;
    }

    /*! Releases ownership to the caller. The KFileDescriptor becomes invalid. */
    [[nodiscard]] int take()
    {
        return std::exchange(m_fd, -1);
    }

    /*! Resets the KFileDescriptor to an invalid state, closing the file descriptor if it is valid. */
    void reset()
    {
        if (m_fd != -1) {
            if (::close(m_fd) != 0) {
                qWarning() << "Failed to close file descriptor:" << strerror(errno);
            }
            m_fd = -1;
        }
    }

    /*! Duplicates the KFileDescriptor and its backing fd. The copy is always CLOEXEC. */
    [[nodiscard]] KFileDescriptor duplicate() const
    {
        return m_fd != -1 ? KFileDescriptor{fcntl(m_fd, F_DUPFD_CLOEXEC, 0)} : KFileDescriptor{};
    }

private:
    int m_fd = -1;
};
