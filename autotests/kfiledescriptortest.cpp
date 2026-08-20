// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QTest>

#include "kfiledescriptor.h"

class KFileDescriptorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInvalid()
    {
        KFileDescriptor invalidFd;
        QVERIFY(!invalidFd.isValid());
    }

    void testValid()
    {
        auto rawFd = fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 0);
        QVERIFY(!isClosed(rawFd)); // sanity check, we'll check the inverse later

        {
            KFileDescriptor fd{rawFd};
            QVERIFY(fd.isValid());
            QCOMPARE(fd.get(), rawFd);

            auto other = fd.duplicate();
            QVERIFY(other.isValid());
            other.reset();
            QVERIFY(fd.isValid());

            auto moved = std::move(fd);
            QVERIFY(moved.isValid());
            QVERIFY(!fd.isValid());
            QCOMPARE(moved.get(), rawFd);
        }

        QVERIFY(isClosed(rawFd));
    }

private:
    bool isClosed(int fd)
    {
        struct stat st{};
        if (fstat(fd, &st) == -1) {
            return errno == EBADF;
        }
        return false;
    }
};

QTEST_MAIN(KFileDescriptorTest)

#include "kfiledescriptortest.moc"
