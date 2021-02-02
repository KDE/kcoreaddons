/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2017 Forrest Smith <forrestthewoods@gmail.com>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed   <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kfuzzymatcher.h"

#include <QString>
#include <QStringView>

/**
 * Custom toLower function which is much faster than
 * c.toLower() directly
 */
static inline constexpr QChar toLower(QChar c)
{
    return c.isLower() ? c : c.toLower();
}

// internal
// clang-format off
static bool match_recursive(QStringView::const_iterator pattern,
                            QStringView::const_iterator str,
                            int &outScore,
                            const QStringView::const_iterator strBegin,
                            const QStringView::const_iterator strEnd,
                            const QStringView::const_iterator patternEnd,
                            const uint8_t *srcMatches,
                            uint8_t *matches,
                            int maxMatches,
                            int nextMatch,
                            int &recursionCount,
                            int seqBonus = 15)
{
    static constexpr int recursionLimit = 10;
    // Count recursions
    ++recursionCount;
    if (recursionCount >= recursionLimit) {
        return false;
    }

    // Detect end of strings
    if (pattern == patternEnd || str == strEnd) {
        return false;
    }

    // Recursion params
    bool recursiveMatch = false;
    uint8_t bestRecursiveMatches[256];
    int bestRecursiveScore = 0;

    // Loop through pattern and str looking for a match
    bool firstMatch = true;
    QChar currentPatternChar = toLower(*pattern);
    while (pattern != patternEnd && str != strEnd) {
        // Found match
        if (currentPatternChar == toLower(*str)) {
            // Supplied matches buffer was too short
            if (nextMatch >= maxMatches) {
                return false;
            }

            // "Copy-on-Write" srcMatches into matches
            if (firstMatch && srcMatches) {
                memcpy(matches, srcMatches, nextMatch);
                firstMatch = false;
            }

            // Recursive call that "skips" this match
            uint8_t recursiveMatches[256];
            int recursiveScore = 0;
            const auto strNextChar = std::next(str);
            if (match_recursive(pattern, strNextChar, recursiveScore, strBegin,
                                strEnd, patternEnd, matches, recursiveMatches,
                                sizeof(recursiveMatches), nextMatch, recursionCount)) {
                // Pick best recursive score
                if (!recursiveMatch || recursiveScore > bestRecursiveScore) {
                    static_assert(sizeof(recursiveMatches) == sizeof(bestRecursiveMatches), "Should be equal");
                    memcpy(bestRecursiveMatches, recursiveMatches, sizeof(recursiveMatches));
                    bestRecursiveScore = recursiveScore;
                }
                recursiveMatch = true;
            }

            // Advance
            matches[nextMatch++] = (uint8_t)(std::distance(strBegin, str));
            ++pattern;
            currentPatternChar = toLower(*pattern);
        }
        ++str;
    }

    // Determine if full pattern was matched
    const bool matched = pattern == patternEnd;

    // Calculate score
    if (matched) {
        const int sequentialBonus = seqBonus; // bonus for adjacent matches
        static constexpr int separatorBonus = 30; // bonus if match occurs after a separator
        static constexpr int camelBonus = 30; // bonus if match is uppercase and prev is lower
        static constexpr int firstLetterBonus = 15; // bonus if the first letter is matched

        static constexpr int leadingLetterPenalty = -5; // penalty applied for every letter in str before the first match
        static constexpr int maxLeadingLetterPenalty = -15; // maximum penalty for leading letters
        static constexpr int unmatchedLetterPenalty = -1; // penalty for every letter that doesn't matter

        // Initialize score
        outScore = 100;

        // Apply leading letter penalty
        const int penalty = std::max(leadingLetterPenalty * matches[0], maxLeadingLetterPenalty);

        outScore += penalty;

        // Apply unmatched penalty
        const int unmatched = (int)(std::distance(strBegin, strEnd)) - nextMatch;
        outScore += unmatchedLetterPenalty * unmatched;

        // Apply ordering bonuses
        for (int i = 0; i < nextMatch; ++i) {
            const uint8_t currIdx = matches[i];

            if (i > 0) {
                const uint8_t prevIdx = matches[i - 1];

                // Sequential
                if (currIdx == (prevIdx + 1)) {
                    outScore += sequentialBonus;
                }
            }

            // Check for bonuses based on neighbor character value
            if (currIdx > 0) {
                // Camel case
                const QChar neighbor = *(strBegin + currIdx - 1);
                const QChar curr = *(strBegin + currIdx);
                if (neighbor.isLower() && curr.isUpper()) {
                    outScore += camelBonus;
                }

                // Separator
                const bool neighborSeparator = neighbor == QLatin1Char('_') || neighbor == QLatin1Char(' ');
                if (neighborSeparator) {
                    outScore += separatorBonus;
                }
            } else {
                // First letter
                outScore += firstLetterBonus;
            }
        }
    }

    // Return best result
    if (recursiveMatch && (!matched || bestRecursiveScore > outScore)) {
        // Recursive score is better than "this"
        memcpy(matches, bestRecursiveMatches, maxMatches);
        outScore = bestRecursiveScore;
        return true;
    } else if (matched) {
        // "this" score is better than recursive
        return true;
    } else {
        // no match
        return false;
    }
}
// clang-format on

static bool match_internal(QStringView pattern, QStringView str, int &outScore, unsigned char *matches, int maxMatches)
{
    int recursionCount = 0;

    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    return match_recursive(patternIt, strIt, outScore, strIt, strEnd, patternEnd,
                           nullptr, matches, maxMatches, 0, recursionCount);
}

/**************************************************************/

QString KFuzzyMatcher::toFuzzyMatchedDisplayString(QStringView pattern, QStringView str, QStringView htmlTag, QStringView htmlTagClose)
{
    bool wasMatching = false;
    QString ret = str.toString();
    for (int i = 0, j = 0; i < ret.size() && j < pattern.size(); ++i) {
        bool matching = ret.at(i).toLower() == pattern.at(j).toLower();
        if (!wasMatching && matching) {
            ret.insert(i, htmlTag);
            i += htmlTag.size();
            ++j;
            wasMatching = true;
        } else if (wasMatching && !matching) {
            ret.insert(i, htmlTagClose);
            i += htmlTagClose.size();
            wasMatching = false;
        } else if (matching) {
            ++j;
        }
    }

    if (wasMatching) {
        ret.append(htmlTagClose);
    }

    return ret;
}

bool KFuzzyMatcher::matchSimple(QStringView pattern, QStringView str)
{
    auto patternIt = pattern.cbegin();
    for (auto strIt = str.cbegin(); strIt != str.cend() && patternIt != pattern.cend(); ++strIt) {
        if (strIt->toLower() == patternIt->toLower()) {
            ++patternIt;
        }
    }
    return patternIt == pattern.cend();
}

KFuzzyMatcher::Result KFuzzyMatcher::matchSequential(QStringView pattern, QStringView str)
{
    int recursionCount = 0;
    uint8_t matches[256];
    auto maxMatches = sizeof(matches);
    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    int score = 0;
    const bool matched = match_recursive(patternIt, strIt, score, strIt, strEnd, patternEnd,
                           nullptr, matches, maxMatches, 0, recursionCount, 40);
    return KFuzzyMatcher::Result{score, matched};
}

KFuzzyMatcher::Result KFuzzyMatcher::match(QStringView pattern, QStringView str)
{
    uint8_t matches[256];
    int score = 0;
    const bool matched = match_internal(pattern, str, score, matches, sizeof(matches));
    return KFuzzyMatcher::Result{score, matched};
}
