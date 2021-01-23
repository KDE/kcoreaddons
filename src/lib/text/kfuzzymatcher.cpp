/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2017 Forrest Smith <forrestthewoods@gmail.com>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed   <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kfuzzymatcher.h"

#include <QString>
#include <QStringView>

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
    while (pattern != patternEnd && str != strEnd) {
        // Found match
        if (pattern->toLower() == str->toLower()) {
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
            int recursiveScore;
            auto strNextChar = std::next(str);
            if (match_recursive(pattern, strNextChar, recursiveScore, strBegin,
                                strEnd, patternEnd, matches, recursiveMatches,
                                sizeof(recursiveMatches), nextMatch, recursionCount)) {
                // Pick best recursive score
                if (!recursiveMatch || recursiveScore > bestRecursiveScore) {
                    memcpy(bestRecursiveMatches, recursiveMatches, 256);
                    bestRecursiveScore = recursiveScore;
                }
                recursiveMatch = true;
            }

            // Advance
            matches[nextMatch++] = (uint8_t)(std::distance(strBegin, str));
            ++pattern;
        }
        ++str;
    }

    // Determine if full pattern was matched
    bool matched = pattern == patternEnd;

    // Calculate score
    if (matched) {
        int sequentialBonus = seqBonus; // bonus for adjacent matches
        static constexpr int separatorBonus = 30; // bonus if match occurs after a separator
        static constexpr int camelBonus = 30; // bonus if match is uppercase and prev is lower
        static constexpr int firstLetterBonus = 15; // bonus if the first letter is matched

        static constexpr int leadingLetterPenalty = -5; // penalty applied for every letter in str before the first match
        static constexpr int maxLeadingLetterPenalty = -15; // maximum penalty for leading letters
        static constexpr int unmatchedLetterPenalty = -1; // penalty for every letter that doesn't matter

        // Iterate str to end
        while (str != strEnd) {
            ++str;
        }

        // Initialize score
        outScore = 100;

        // Apply leading letter penalty
        int penalty = leadingLetterPenalty * matches[0];
        if (penalty < maxLeadingLetterPenalty) {
            penalty = maxLeadingLetterPenalty;
        }

        outScore += penalty;

        // Apply unmatched penalty
        const int unmatched = (int)(std::distance(strBegin, str)) - nextMatch;
        outScore += unmatchedLetterPenalty * unmatched;

        // Apply ordering bonuses
        for (int i = 0; i < nextMatch; ++i) {
            uint8_t currIdx = matches[i];

            if (i > 0) {
                uint8_t prevIdx = matches[i - 1];

                // Sequential
                if (currIdx == (prevIdx + 1)) {
                    outScore += sequentialBonus;
                }
            }

            // Check for bonuses based on neighbor character value
            if (currIdx > 0) {
                // Camel case
                QChar neighbor = *(strBegin + currIdx - 1);
                QChar curr = *(strBegin + currIdx);
                if (neighbor.isLower() && curr.isUpper())
                    outScore += camelBonus;

                // Separator
                bool neighborSeparator = neighbor == QLatin1Char('_') || neighbor == QLatin1Char(' ');
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

static bool match_internal(const QStringView pattern, const QStringView str, int &outScore, unsigned char *matches, int maxMatches)
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

QString KFuzzyMatcher::toFuzzyMatchedDisplayString(const QStringView pattern, QString &str, const QString &htmlTag, const QString &htmlTagClose)
{
    // TODO: improve this so that we don't have to put html tags on every char
    // probably using some kind of intervals container
    int j = 0;
    for (int i = 0; i < str.size() && j < pattern.size(); ++i) {
        if (str.at(i).toLower() == pattern.at(j).toLower()) {
            str.replace(i, 1, htmlTag + str.at(i) + htmlTagClose);
            i += htmlTag.size() + htmlTagClose.size();
            ++j;
        }
    }
    return str;
}

bool KFuzzyMatcher::matchSimple(const QStringView pattern, const QStringView str)
{
    auto patternIt = pattern.cbegin();
    for (auto strIt = str.cbegin(); strIt != str.cend() && patternIt != pattern.cend(); ++strIt) {
        if (strIt->toLower() == patternIt->toLower()) {
            ++patternIt;
        }
    }
    return patternIt == pattern.cend();
}

bool KFuzzyMatcher::matchSequential(const QStringView pattern, const QStringView str, int &outScore)
{
    int recursionCount = 0;
    uint8_t matches[256];
    auto maxMatches = sizeof(matches);
    auto strIt = str.cbegin();
    auto patternIt = pattern.cbegin();
    const auto patternEnd = pattern.cend();
    const auto strEnd = str.cend();

    return match_recursive(patternIt, strIt, outScore, strIt, strEnd, patternEnd,
                           nullptr, matches, maxMatches, 0, recursionCount, 40);
}

bool KFuzzyMatcher::match(const QStringView pattern, const QStringView str, int &outScore)
{
    uint8_t matches[256];
    return match_internal(pattern, str, outScore, matches, sizeof(matches));
}
