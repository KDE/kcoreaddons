/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QList>
#include <QString>

#include "krandomsequence.h"
#include "krandom.h"

#include <stdio.h>

int
main(/*int argc, char *argv[]*/)
{
    long seed;
    KRandomSequence seq;

    seed = 2;
    seq.setSeed(seed); printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seed = 0;
    seq.setSeed(seed); printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seed = 0;
    seq.setSeed(seed); printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seed = 2;
    seq.setSeed(seed); printf("Seed = %4ld :", seed);
    for (int i = 0; i < 20; i++) {
        printf("%3ld ", seq.getLong(100));
    }
    printf("\n");

    seq.setSeed(KRandom::random());

    QList<QString> list;
    list.append(QLatin1String("A"));
    list.append(QLatin1String("B"));
    list.append(QLatin1String("C"));
    list.append(QLatin1String("D"));
    list.append(QLatin1String("E"));
    list.append(QLatin1String("F"));
    list.append(QLatin1String("G"));

    for (QList<QString>::Iterator str = list.begin(); str != list.end(); ++str) {
        printf("%s", str->toLatin1().data());
    }
    printf("\n");

    seq.randomize(list);

    for (QList<QString>::Iterator str = list.begin(); str != list.end(); ++str) {
        printf("%s", str->toLatin1().data());
    }
    printf("\n");

    seq.randomize(list);

    for (QList<QString>::Iterator str = list.begin(); str != list.end(); ++str) {
        printf("%s", str->toLatin1().data());
    }
    printf("\n");

    seq.randomize(list);

    for (QList<QString>::Iterator str = list.begin(); str != list.end(); ++str) {
        printf("%s", str->toLatin1().data());
    }
    printf("\n");
}
