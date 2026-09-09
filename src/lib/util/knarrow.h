// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Sune Vuorela <sune@vuorela.dk>
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#pragma once

#include <exception>
#include <type_traits>
#if defined(__cpp_lib_expected) || defined(Q_QDOC)
#include <expected>
#endif

#include <QDebug>

// All KF6 public headers should provide the version macros.
// Include version header explicitly because the export header
// (which usually takes care of that) isn't used here
#include "kcoreaddons_version.h" // IWYU pragma: export

/*!
    \namespace KNarrow
    \inmodule KCoreAddons
    \since 6.33

    \brief Narrowing conversion utilities

    This namespace provides utilities for performing narrowing conversions safely.
    If you have C++23 enabled, which is recommended, then you can use \l expectingNarrow.
    If you do not have C++23 or want to abort you can use \l narrow.

    \code
    // C++23
    {
        auto u = KNarrow::expectingNarrow<unsigned int>(30); // OK, a value of 30 fits in an unsigned
        if (u.has_value()) {
            // Use the value
        }
        auto i = KNarrow::expectingNarrow<int>(std::numeric_limits<long>::max()); // Bad, a value of long::max does not fit in an int
        if (i.has_value()) {
            // Use the value. It won't have one though :)
        }
    }

    // Always available
    {
        auto u = KNarrow::narrow<unsigned int>(30); // OK
        auto i = KNarrow::narrow<int>(std::numeric_limits<long>::max()); // aborts or throws
    }
    \endcode
*/
namespace KNarrow
{

/*!
    \enum KNarrow::NarrowingErrorType
    \brief Types of narrowing errors

    \value ValueChanged The value changed during narrowing (e.g. uint64::max to
        uint32 would yield different values)
    \value ValueMeaningChanged The value technically is the same but its domain
        meaning has changed (e.g. uint8::max to int8 would be the same value in
        their respective domains but means different things: 255 versus -1
        depending on which representation you compare with)
*/
enum class NarrowingErrorType {
    ValueChanged,
    ValueMeaningChanged,
};

/*!
    \class KNarrow::NarrowingError
    \brief Exception type for narrowing errors
*/
struct NarrowingError : public std::exception {
    explicit NarrowingError(NarrowingErrorType type)
        : m_type(type)
    {
    }

    /*! \return the type of narrowing error that occurred */
    [[nodiscard]] NarrowingErrorType type() const noexcept
    {
        return m_type;
    }

    /*! \return a human-readable description of the narrowing error */
    [[nodiscard]] const char *what() const noexcept override
    {
        return "KNarrow::NarrowingError";
    }

private:
    NarrowingErrorType m_type;
};

#if defined(__cpp_lib_expected) || defined(Q_QDOC)
/*!
    Narrow Input into Output type and return a \l {https://en.cppreference.com/cpp/utility/expected} {std::expected}.
    Similar to gsl::narrow().

    \param i The input value to narrow

    \return The narrowed value of type Output. When the value is not compatible
        with the target type a std::unexpected containing the \l KNarrow::NarrowingErrorType is returned.

    \note This function requires C++23 support for std::expected.

    \warning The documentation may incorrectly render the return type as \e {int}.
        It is in fact \e {std::expected<Output, NarrowingErrorType>}
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
*/
template<typename Output, typename Input>
[[nodiscard]] Output narrow(Input i)
{
    Output o = i;

    if (i != Input(o)) {
#ifdef __cpp_exceptions
        throw NarrowingError{NarrowingErrorType::ValueChanged};
#else
        qCritical() << "Narrowing conversion from" << typeid(Input).name() << "to" << typeid(Output).name() << "failed: value" << i << "changed to" << o;
        std::abort();
#endif
    }

    if (const auto sameSignedness = (std::is_signed_v<Input> && std::is_signed_v<Output>); !sameSignedness && ((i < Input{}) != (o < Output{}))) {
#ifdef __cpp_exceptions
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
