#ifndef KFUZZYMATCHERTEST_H
#define KFUZZYMATCHERTEST_H

#include <QObject>

class KFuzzyMatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMatchSimple();
    void testMatch_data();
    void testMatch();
    void testMatchSequential_data();
    void testMatchSequential();
    void testToFuzzyMatchedDisplayString_data();
    void testToFuzzyMatchedDisplayString();
};

#endif // KFUZZYMATCHERTEST_H
