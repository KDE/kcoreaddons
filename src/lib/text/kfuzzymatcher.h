/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2017 Forrest Smith <forrestthewoods@gmail.com>
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <kcoreaddons_export.h>

class QString;
class QStringView;

/**
 * This code is ported to Qt from lib_fts:
 * https://github.com/forrestthewoods/lib_fts
 * which tries to replicate SublimeText like fuzzy matching.
 *
 * @note
 * In general match() and matchSequential() will suffice for most usecases
 * but should the need arise to extend this for some particular reason, for example
 * scoring based on a different kind of separator, it should ideally be handled in a
 * separate internal method with an interface similar to the existing ones.
 *
 * All methods in here are stateless i.e., the input string will not be modified. Also
 * note that strings in all the functions in this namespace will be matched case-insensitively.
 *
 * If you are using this with @c QSortFilterProxyModel, and you choose to use match()
 * or matchSequential(), you need override both @c QSortFilterProxyModel::lessThan and
 * @c QSortFilterProxyModel::filterAcceptsRow. A simple example:
 *
 * \code
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (m_pattern.isEmpty()) {
            return true;
        }

        int score = 0;
        const auto idx = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto actionName = idx.data().toString().splitRef(QLatin1Char(':')).at(1);
        const bool res = kfts::fuzzy_match_sequential(m_pattern, actionName, score);
        // store the score in the source model
        sourceModel()->setData(idx, score, ScoreRole);
        return res;
    }

    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override
    {
        // use the score here to sort
        const int l = sourceLeft.data(ScoreRole).toInt();
        const int r = sourceRight.data(ScoreRole).toInt();
        return l < r;
    }
 * \endcode
 *
 * Additionally you must not use @c invalidateFilter() if you go with the above approach. Instead
 * use @c beginResetModel()/@c endResetModel():
 *
 * \code
 *  Q_SLOT void setFilterString(const QString &string)
    {
        beginResetModel();
        m_pattern = string;
        endResetModel();
    }
 * \endcode
 *
 * @short Namespace for fuzzy matching of strings
 * @author Waqar Ahmed <waqar.17a@gmail.com>
 */
namespace KFuzzyMatcher
{

/**
 * @brief The result of a fuzzy match
 */
struct KCOREADDONS_EXPORT Result
{
    /** Score of this match. This can be negative. if matched is @c false
        then the score will be zero.
    */
    int score;
    /** @c true if match was successful */
    bool matched;
};

/**
 * @brief Simple fuzzy matching of chars in @p pattern with chars in @p str
 * sequentially. If there is a match, it will return true and false otherwise.
 * There is no scoring. You should use this if score is not important for you
 * and only matching is important.
 *
 * @param pattern to search for. For e.g., text entered by a user to filter a
 * list
 * @param str the current string from your list of strings
 * @return @c true on sucessful match
 *
 * @since 5.79
 */
KCOREADDONS_EXPORT bool matchSimple(QStringView pattern, QStringView str);

/**
 * @brief This should be the function to use if the strings you are matching
 * consist of file names, code etc.
 *
 * The return value of this function contains Result#score which should be used to
 * sort the results. Without sorting of the results this function won't very effective.
 * Also note that this function scores separator matches higher than sequential matches.
 * See matchSequential() for more details
 *
 * If you are using this function in a @c QSortFilterProxyModel based class, keep
 * in mind that during the initial load of the model when the pattern is empty,
 * this function will return false and as a result your "view" will be empty. To
 * prevent this, you should place a check in the beginning of your function
 * which returns true if the pattern is empty.
 *
 * @param pattern to search for. For e.g., text entered by a user to filter a
 * list or model
 * @param str the current string from your list of strings
 * @return A Result type with score of this match and whether the match was
 * successful. If there is no match, score is zero. If the match is successful,
 * score must be used to sort the results.
 *
 * @since 5.79
 */
KCOREADDONS_EXPORT Result match(QStringView pattern, QStringView str);

/**
 * @brief This is a special case function which scores sequential matches
 * higher than separator matches. For example, if you have the following
 * strings:
 *
 * \code
 * "Sort Items"
 * "Split or join a tag"
 * \endcode
 *
 * and you use match with pattern @c "sort", then it can be that the second
 * result is scored better than the first one despite the fact that the first one
 * is an exact match. This happens because match will score separator matches
 * higher than sequential matches. Use this function if you want sequential matches
 * to take preference. This can be useful if you are filtering user interface
 * strings
 *
 * @param pattern to search for. For e.g., text entered by a user to filter a
 * list or model
 * @param str the current string from your list of strings
 * @return A Result type with score of this match and whether the match was
 * successful. If there is no match, score is zero. If the match is successful,
 * score must be used to sort the results.
 *
 * @since 5.79
 */
KCOREADDONS_EXPORT Result matchSequential(QStringView pattern, QStringView str);

/**
 * @brief This is a utility function to display what is being matched.
 * You can use the output of this function with a @c QTextDocument
 * or in a @c QStyledItemDelegate.
 *
 * Example:
 * \code
 * pattern = "kate", str = "kateapp", htmlTag = "<b>"
 * output =  <b>kate</b>app
 * \endcode
 *
 * @param pattern is the current pattern entered by user
 * @param str is the string that will be wrapped with @p htmlTag.
 * @param htmlTag is the html tag you want to use for example \<b\> or \<span
 * style=...\>
 * @param htmlTagClose is the corresponding closing tag for @p htmlTag. The
 * function does not check whether it is a closing tag for @p htmlTag or
 * not.
 * @return html styled output string
 *
 * @since 5.79
 */
KCOREADDONS_EXPORT QString toFuzzyMatchedDisplayString(QStringView pattern,
                                                       QStringView str,
                                                       QStringView htmlTag,
                                                       QStringView htmlTagClose);

} // namespace KFuzzyMatcher
