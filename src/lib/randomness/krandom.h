/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 2000 Charles Samuels <charles@kde.org>
    SPDX-FileCopyrightText: 2005 Joseph Wenninger <kde@jowenn.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRANDOM_H
#define KRANDOM_H

#include <kcoreaddons_export.h>

#include <QString>

/**
 * \headerfile krandom.h <KRandom>
 *
 * @short Helper class to create random data
 *
 * This namespace provides methods which generate random data.
 * KRandom is not recommended for serious random-number generation needs,
 * like cryptography.
 */
namespace KRandom
{
/**
 * Generates a uniform random number.
 * @return A random number in the range [0, RAND_MAX). The RNG is seeded
 *   on first use.
 */
KCOREADDONS_EXPORT int random();

/**
 * Generates a random string.  It operates in the range [A-Za-z0-9]
 * @param length Generate a string of this length.
 * @return the random string
 */
KCOREADDONS_EXPORT QString randomString(int length);
}

#endif

