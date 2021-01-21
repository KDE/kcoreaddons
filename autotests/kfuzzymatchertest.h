#ifndef KFUZZYMATCHERTEST_H
#define KFUZZYMATCHERTEST_H

#include <QObject>

class KFuzzyMatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMatchSimple();
    void testMatch();
    void testMatch2();
    void testMatchSequential();
    void testToFuzzyMatchedDisplayString();
};

#endif // KFUZZYMATCHERTEST_H
