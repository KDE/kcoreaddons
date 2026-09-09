// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Sune Vuorela <sune@vuorela.dk>
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#pragma once

#include <exception>
#include <type_traits>
#if defined(__cpp_lib_expected)
#include <expected>
#endif

#include <QDebug>

/*! \brief Narrowing conversion utilities */
namespace KNarrow
{

enum class NarrowingErrorType {
    /*! The value changed during narrowing (e.g. uint64::max to uint32 would yield different values) */
    ValueChanged,
    /*!
        The value technically is the same but its domain meaning has changed
        (e.g. uint8::max to int8 would be the same value in their respective domains but means different things:
        255 versus -1 depending on which representation you compare with).
    */
    ValueMeaningChanged,
};

struct NarrowingError : public std::exception {
    explicit NarrowingError(NarrowingErrorType type)
        : m_type(type)
    {
    }

    [[nodiscard]] NarrowingErrorType type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] const char *what() const noexcept override
    {
        return "KNarrow::NarrowingError";
    }

private:
    NarrowingErrorType m_type;
};

#if defined(__cpp_lib_expected)
/*!
    Narrow Input into Output type and return a std::expected. Similar to gsl::narrow().

    \param i The input value to narrow

    \return The narrowed value of type Output. When the value is not compatible
    with the target type a std::unexpected is returned.

    \code
    auto u = KNarrow::expectingNarrow<unsigned int>(30); // OK, a value of 30 fits in an unsigned
    auto i = KNarrow::expectingNarrow<int>(std::numeric_limits<long>::max()); // Bad, a value of long::max does not fit in an int
    \endcode
*/
template<typename Output, typename Input>
[[nodiscard]] std::expected<Output, NarrowingErrorType> expectingNarrow(Input i)
{
    Output o = i;

    if (i != Input(o)) {
        return std::unexpected(NarrowingErrorType::ValueChanged);
    }

    if (const auto sameSignedness = (std::is_signed_v<Input> && std::is_signed_v<Output>); !sameSignedness && ((i < Input{}) != (o < Output{}))) {
        return std::unexpected(NarrowingErrorType::ValueMeaningChanged);
    }

    return o;
}
#endif

/*!
    Narrow Input into Output type and abort if the value changed. Similar to gsl::narrow().

    \param i The input value to narrow

    \return The narrowed value of type Output. When the value is not compatible
    with the target type an exception is thrown (if enabled), or the program aborts.

    \code
    auto u = KNarrow::narrow<unsigned int>(30); // OK, a value of 30 fits in an unsigned
    auto i = KNarrow::narrow<int>(std::numeric_limits<long>::max()); // Bad, a value of long::max does not fit in an int
    \endcode
*/
template<typename Output, typename Input>
[[nodiscard]] Output narrow(Input i)
{
    Output o = i;

    if (i != Input(o)) {
#if defined(__cpp_exceptions)
        throw NarrowingError{NarrowingErrorType::ValueChanged};
#else
        qCritical() << "Narrowing conversion from" << typeid(Input).name() << "to" << typeid(Output).name() << "failed: value" << i << "changed to" << o;
        std::abort();
#endif
    }

    if (const auto sameSignedness = (std::is_signed_v<Input> && std::is_signed_v<Output>); !sameSignedness && ((i < Input{}) != (o < Output{}))) {
#if defined(__cpp_exceptions)
        throw NarrowingError{NarrowingErrorType::ValueMeaningChanged};
#else
        qCritical() << "Narrowing conversion from" << typeid(Input).name() << "to" << typeid(Output).name() << "failed: value" << i
                    << "is out of range for the target type";
        std::abort();
#endif
    }

    return o;
}

} // namespace KNarrow
