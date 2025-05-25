/*
    SPDX-FileCopyrightText: 2017 James D. Smith <smithjd15@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "config-tests.h"
#include "filesystemmetadata.h"
#include <qstandardpaths.h>

#include <QFile>
#include <QTest>

namespace
{
const auto TEST_FILENAME = QStringLiteral("writertest-usermetadata.txt");
const auto TEST_SYMLINK = QStringLiteral("dangling_symlink-metadata");

QString testOutputPath(const QString &fileName)
{
    return QStringLiteral(TESTS_OUTPUT_PATH "/%1").arg(fileName);
}
}

class FilesystemMetaDataWriterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void test();
    void testMissingPermision();
    void testMetadataSize();
    void testMetadataNameTooLong();
    void testDanglingSymlink();
    void testRemoveMetadata();
    void testMetadataFolder();
    void cleanupTestCase();

private:
    QFile m_writerTestFile;
};

void FilesystemMetaDataWriterTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    QVERIFY(QDir().mkpath(testOutputPath({})));

    m_writerTestFile.setFileName(testOutputPath(TEST_FILENAME));
    auto opened = m_writerTestFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QVERIFY(opened);

    QFile::link(testOutputPath(QStringLiteral("invalid_target")), testOutputPath(TEST_SYMLINK));
}

void FilesystemMetaDataWriterTest::cleanupTestCase()
{
    m_writerTestFile.remove();
    QFile(testOutputPath(TEST_SYMLINK)).remove();
}

void FilesystemMetaDataWriterTest::testMissingPermision()
{
#ifdef Q_OS_WIN
    QSKIP("Only unix permissions can restrict metadata writing");
#endif
    m_writerTestFile.setPermissions(QFileDevice::ReadOwner);
    auto opt = FilesystemMetaData::forFile(testOutputPath(TEST_FILENAME));
    QVERIFY(opt.has_value());
    FilesystemMetaData md = *opt;

    auto result = md.setAttribute(QStringLiteral("test"), QStringLiteral("my-value").toUtf8());
    QCOMPARE(result, FilesystemMetaData::MissingPermission);

    QVERIFY(m_writerTestFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner));
}

void FilesystemMetaDataWriterTest::testMetadataSize()
{
    auto opt = FilesystemMetaData::forFile(testOutputPath(TEST_FILENAME));
    QVERIFY(opt.has_value());
    FilesystemMetaData md = *opt;

    // In the current ext2, ext3, and ext4 filesystem implementations,
    // the total bytes used by the names and values of all of a file's
    // extended attributes must fit in a single filesystem block (1/2/4 kB)

    // all implementations should support at least 512 B
    const auto smallSize = 512; // 512 B
    auto smallValue = QByteArray(smallSize, 'a');
    auto result = md.setAttribute(QStringLiteral("test"), smallValue);
    QCOMPARE(result, FilesystemMetaData::NoError);
    QCOMPARE(md.attribute(QStringLiteral("test")), smallValue);

    // a big value, equal to the maximum value of an extended attribute according to Linux VFS
    // applies to XFS, btrfs...
    auto maxSize = 64 * 1024;
    const auto bigValue = QByteArray(maxSize, 'a'); // 64 kB
    result = md.setAttribute(QStringLiteral("test"), bigValue);
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD) || defined(Q_OS_WIN)
    // BSD VFS has no such limit to 64 kB
    QCOMPARE(result, FilesystemMetaData::NoError);
    QCOMPARE(md.attribute(QStringLiteral("test")), bigValue);
#else
    QCOMPARE(result, FilesystemMetaData::NoSpace);
#endif

    // In Linux, The VFS-imposed limits on attribute names and
    // values are 255 bytes and 64 kB, respectively.
    auto excessiveValue = QByteArray(maxSize + 1, 'a');
    result = md.setAttribute(QStringLiteral("test"), excessiveValue);
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD) || defined(Q_OS_WIN)
    QCOMPARE(result, FilesystemMetaData::NoError);
    QCOMPARE(md.attribute(QStringLiteral("test")), excessiveValue);
#else
    // In Linux, we exceed the max value of an extended attribute, the error is different
    QCOMPARE(result, FilesystemMetaData::ValueTooBig);
#endif
}

void FilesystemMetaDataWriterTest::testMetadataNameTooLong()
{
    auto testFile = testOutputPath(TEST_FILENAME);
    auto opt = FilesystemMetaData::forFile(testFile);
    QVERIFY(opt.has_value());
    FilesystemMetaData md = *opt;

    // BSD and Linux have a limit of the attribute name of 255 bytes
    // Windows has by default a limit on filename that applies to filesystem metadata
    auto longName = QString(256, QLatin1Char('a'));
    int result = md.setAttribute(longName, QStringLiteral("smallValue").toUtf8());
    QCOMPARE(result, FilesystemMetaData::NameToolong);
}

void FilesystemMetaDataWriterTest::test()
{
    auto testFile = testOutputPath(TEST_FILENAME);
    auto opt = FilesystemMetaData::forFile(testFile);
    QVERIFY(opt.has_value());
    FilesystemMetaData md = *opt;

    QStringList expected;

    // Attribute
    md.setAttribute(QStringLiteral("test.attribute"), QStringLiteral("attribute").toUtf8());
    QCOMPARE(md.attribute(QStringLiteral("test.attribute")), QStringLiteral("attribute").toUtf8());
    expected = QStringList{QStringLiteral("test.attribute")};
    QCOMPARE(md.attributes(), expected);
    md.setAttribute(QStringLiteral("test.attribute2"), QStringLiteral("attribute2").toUtf8());
    QCOMPARE(md.attribute(QStringLiteral("test.attribute2")), QStringLiteral("attribute2").toUtf8());
    QCOMPARE(md.attributes().length(), 2);

    md.setAttribute(QStringLiteral("test.attribute"), QByteArray());
    QVERIFY(!md.hasAttribute(QStringLiteral("test.attribute")));
    expected = QStringList{QStringLiteral("test.attribute2")};
    QCOMPARE(md.attributes(), expected);
    md.setAttribute(QStringLiteral("test.attribute2"), QByteArray());
    QVERIFY(!md.hasAttribute(QStringLiteral("test.attribute2")));
    QCOMPARE(md.attributes(), {});

    // Check for side effects of calling sequence
    QVERIFY(!md.hasAttribute(QStringLiteral("test.check_contains")));
    md.setAttribute(QStringLiteral("test.check_contains"), QStringLiteral("dummy").toUtf8());
    QVERIFY(md.hasAttribute(QStringLiteral("test.check_contains")));
    md.setAttribute(QStringLiteral("test.check_contains"), QByteArray());
    QVERIFY(!md.hasAttribute(QStringLiteral("test.check_contains")));
}

void FilesystemMetaDataWriterTest::testDanglingSymlink()
{
    auto opt = FilesystemMetaData::forFile(testOutputPath(TEST_SYMLINK));
    QVERIFY(opt.has_value());
    FilesystemMetaData md = *opt;
    QVERIFY(md.attributes().isEmpty());
}

void FilesystemMetaDataWriterTest::testRemoveMetadata()
{
    auto testFile = testOutputPath(TEST_FILENAME);
    auto opt = FilesystemMetaData::forFile(testFile);
    QVERIFY(opt.has_value());
    FilesystemMetaData md = *opt;

    const auto tagValue = QStringLiteral("this/is/a/test/tag").toUtf8();
    QCOMPARE(md.setAttribute(QStringLiteral("tag"), tagValue), FilesystemMetaData::NoError);
    QVERIFY(md.hasAttribute(QStringLiteral("tag")));

    QCOMPARE(md.setAttribute(QStringLiteral("tag"), QByteArray{}), FilesystemMetaData::NoError);
    QVERIFY(!md.hasAttribute(QStringLiteral("tag")));
}

void FilesystemMetaDataWriterTest::testMetadataFolder()
{
    const auto dirPath = testOutputPath(QStringLiteral("metadata-dir"));
    QVERIFY(QDir().mkdir(dirPath));

    auto opt = FilesystemMetaData::forFile(dirPath);
    QVERIFY(opt.has_value());
    FilesystemMetaData md = *opt;

    const auto tagValue = QStringLiteral("this/is/a/test/tag").toUtf8();
    QCOMPARE(md.setAttribute(QStringLiteral("tag"), tagValue), FilesystemMetaData::NoError);
    QVERIFY(md.hasAttribute(QStringLiteral("tag")));

    QCOMPARE(md.setAttribute(QStringLiteral("tag"), QByteArray{}), FilesystemMetaData::NoError);
    QVERIFY(!md.hasAttribute(QStringLiteral("tag")));

    QVERIFY(QDir().rmdir(dirPath));
}

QTEST_GUILESS_MAIN(FilesystemMetaDataWriterTest)

#include "filesystemmetadatawritertest.moc"
