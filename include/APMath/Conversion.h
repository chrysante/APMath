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

namespace APMath {

class APInt;
class APFloat;

/// Convert value \p from to the value of type `To` with the same bit
/// representation as \p from
template <typename To, typename From>
To bitcast(From const& from); // Undefined

template <>
APInt bitcast(APFloat const& from);

template <>
APFloat bitcast(APInt const& from);

} // namespace APMath

#endif // APMATH_CONVERSION_H_
