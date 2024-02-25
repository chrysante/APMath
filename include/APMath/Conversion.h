#ifndef APMATH_CONVERSION_H_
#define APMATH_CONVERSION_H_

#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include <APMath/API.h>

namespace APMath {

class APInt;
class APFloat;

/// Convert value \p from to the value of type `To` with the same bit
/// representation as \p from
template <typename To, typename From>
To bitcast(From const& from); // Undefined

template <>
APMATH_API APInt bitcast(APFloat const& from);

template <>
APMATH_API APFloat bitcast(APInt const& from);

/// Convert \p from to type `To`. \p from is interpreted as `unsigned`
template <typename To, typename From>
To valuecast(From const& from, size_t toBitwidth); // Undefined

template <>
APMATH_API APInt valuecast(APFloat const& from, size_t toBitwidth);

template <>
APMATH_API APFloat valuecast(APInt const& from, size_t toBitwidth);

/// Convert \p from to type `To`. \p from is interpreted as `signed`
template <typename To, typename From>
To signedValuecast(From const& from, size_t toBitwidth); // Undefined

template <>
APMATH_API APInt signedValuecast(APFloat const& from, size_t toBitwidth);

template <>
APMATH_API APFloat signedValuecast(APInt const& from, size_t toBitwidth);

} // namespace APMath

#endif // APMATH_CONVERSION_H_
