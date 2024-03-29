#ifndef APMATH_APFLOAT_H_
#define APMATH_APFLOAT_H_

#include <climits>
#include <compare>
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

class APFloat;

/// APFloat precision structure.
struct APMATH_API APFloatPrec {
    /// Static constructor
    /// \Returns the instance that represents single precision
    static APFloatPrec Single();

    /// Static constructor
    /// \Returns the instance that represents double precision
    static APFloatPrec Double();

    /// The number of bits used to represent the mantissa.
    /// This is equal to `totalBitwidth() - exponentWidth - 1`
    std::size_t mantissaWidth;

    /// The number of bits used to represent the exponent.
    /// This is equal to `totalBitwidth() - mantissaWidth - 1`
    std::size_t exponentWidth;

    /// \Returns `mantissaWidth + exponentWidth + 1` (+1 for the sign bit)
    std::size_t totalBitwidth() const;

    bool operator==(APFloatPrec const&) const = default;
};

/// ## Basic operations

/// Compute sum of \p lhs and \p rhs
APMATH_API APFloat add(APFloat lhs, APFloat const& rhs);

/// Compute difference of \p lhs and \p rhs
APMATH_API APFloat sub(APFloat lhs, APFloat const& rhs);

/// Compute product of \p lhs and \p rhs
APMATH_API APFloat mul(APFloat const& lhs, APFloat const& rhs);

/// Compute quotient of \p lhs and \p rhs
APMATH_API APFloat div(APFloat const& lhs, APFloat const& rhs);

/// Flip sign of \p operand
APMATH_API APFloat negate(APFloat operand);

/// Set precision of \p operand to \p precision
APMATH_API APFloat precisionCast(APFloat operand, APFloatPrec precision);

/// Compare \p lhs and \p rhs
APMATH_API int cmp(APFloat const& lhs, APFloat const& rhs);

/// Compare \p lhs and \p rhs
APMATH_API int cmp(APFloat const& lhs, double rhs);

/// ## Common math functions

/// Compute absolute value of \p arg
APMATH_API APFloat abs(APFloat const& arg);

/// Compute _e_ raised to the power of \p arg
APMATH_API APFloat exp(APFloat const& arg);

/// Compute 2 raised to the power of \p arg
APMATH_API APFloat exp2(APFloat const& arg);

/// Compute 10 raised to the power of \p arg
APMATH_API APFloat exp10(APFloat const& arg);

/// Compute the natural logarithm of \p arg
APMATH_API APFloat log(APFloat const& arg);

/// Compute the logarithm base 2 of \p arg
APMATH_API APFloat log2(APFloat const& arg);

/// Compute the logarithm base 10 of \p arg
APMATH_API APFloat log10(APFloat const& arg);

/// Compute \p base raised to the power of \p exp
APMATH_API APFloat pow(APFloat const& base, APFloat const& exp);

/// Compute square root of \p arg
APMATH_API APFloat sqrt(APFloat const& arg);

/// Compute cube root of \p arg
APMATH_API APFloat cbrt(APFloat const& arg);

/// Compute `hypot` of arguments
APMATH_API APFloat hypot(APFloat const& a, APFloat const& b);

/// Compute sine of \p arg
APMATH_API APFloat sin(APFloat const& arg);

/// Compute cosine of \p arg
APMATH_API APFloat cos(APFloat const& arg);

/// Compute tangent of \p arg
APMATH_API APFloat tan(APFloat const& arg);

/// Compute arc sine of \p arg
APMATH_API APFloat asin(APFloat const& arg);

/// Compute arc cosine of \p arg
APMATH_API APFloat acos(APFloat const& arg);

/// Compute arc tangent of \p arg
APMATH_API APFloat atan(APFloat const& arg);

/// ## class APFloat

/// Arbitraty precision floating point type.
/// Mantissa and exponent width is specified on construction and can be modified
/// with `setPrecision()` Operations involving multiple `APFloat`s usually
/// require the operands to be of same precision. \Note Right now this is
/// implemented in terms of hardware float operations on `float` or `double`
/// type. This is temporary until we take the time to implement proper IEEE
/// emulation. Until then only single and double precision are supported.
class APMATH_API APFloat {
    using Limb = std::uint64_t;

public:
    /// Construct an `APFloat` with `Double` precision
    APFloat(): APFloat(APFloatPrec::Double()) {}

    /// Construct an `APFloat` with \p precision
    explicit APFloat(APFloatPrec precision);

    /// Construct an `APFloat` with \p precision and set it to \p value
    explicit APFloat(long double value, APFloatPrec precision);

    APFloat(APFloat const& rhs);
    APFloat(APFloat&& rhs) noexcept;
    APFloat& operator=(APFloat const& rhs);
    APFloat& operator=(APFloat&& rhs) noexcept;
    ~APFloat();

    /// Swap `*this` and \p rhs
    void swap(APFloat& rhs) noexcept;

    /// `*this += rhs`
    APFloat& add(APFloat const& rhs);

    /// `*this -= rhs`
    APFloat& sub(APFloat const& rhs);

    /// `*this *= rhs`
    APFloat& mul(APFloat const& rhs);

    /// `*this /= rhs`
    APFloat& div(APFloat const& rhs);

    /// `*this = -*this`
    APFloat& negate();

    /// Set precision to \p precision
    APFloat& setPrecision(APFloatPrec precision);

    /// Compare `*this` and \p rhs
    int cmp(APFloat const& rhs) const;

    /// Compare `*this` and \p rhs
    int cmp(double rhs) const;

    /// \Returns The sign of the value
    int signbit() const;

    /// \Returns `true` if the value is `+/-inf`
    bool isInf() const;

    /// \Returns `true` if the value is any `NaN`
    bool isNaN() const;

    /// The bitwidth of this value.
    APFloatPrec precision() const { return { _mantWidth, _expWidth }; }

    /// Convert `*this` to a string in the specified base.
    std::string toString() const;

    /// View over limbs
    ///
    /// \Note Right now this always return a span of size 1
    std::span<Limb const> limbs() const { return { &_limb, 1 }; }

    /// Convert to native float type
    template <typename T>
    T to() const {
        static_assert(std::is_floating_point_v<T>);
        if (isSingle()) {
            return static_cast<T>(_f32);
        }
        else {
            return static_cast<T>(_f64);
        }
    }

    /// Note that this is meant for use with unordered containers and is not a
    /// cryptographic hash.
    std::size_t hash() const;

    /// Try to convert \p str to `APFloat`
    /// \param str All characters except ones representing digits in the
    /// specified base, an initial '-' and the first '.' are ignored.
    static std::optional<APFloat> parse(
        std::string_view str, APFloatPrec precision = APFloatPrec::Double());

    /// Compare for equality.
    bool operator==(APFloat const& rhs) const { return cmp(rhs) == 0; }

    /// \overload
    bool operator==(double rhs) const { return cmp(rhs) == 0; }

    /// Relational comparison.
    std::strong_ordering operator<=>(APFloat const& rhs) const {
        int const res = cmp(rhs);
        if (res < 0) {
            return std::strong_ordering::less;
        }
        if (res > 0) {
            return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

private:
    bool isSingle() const { return precision() == APFloatPrec::Single(); }

private:
    std::uint32_t _mantWidth;
    std::uint32_t _expWidth;
    union {
        float _f32;
        double _f64;
        Limb _limb;
    };
};

} // namespace APMath

template <>
struct std::hash<APMath::APFloat> {
    std::size_t operator()(APMath::APFloat const& value) const {
        return value.hash();
    }
};

#endif // APMATH_APFLOAT_H_
