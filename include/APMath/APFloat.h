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

namespace APMath {

class APFloat;

/// APFloat precision structure.
struct APFloatPrec {
    static APFloatPrec const Single;
    static APFloatPrec const Double;

    std::size_t mantissaWidth;
    std::size_t exponentWidth;

    bool operator==(APFloatPrec const&) const = default;
};

/// ## Basic operations

/// Compute sum of \p lhs and \p rhs
APFloat add(APFloat lhs, APFloat const& rhs);

/// Compute difference of \p lhs and \p rhs
APFloat sub(APFloat lhs, APFloat const& rhs);

/// Compute product of \p lhs and \p rhs
APFloat mul(APFloat const& lhs, APFloat const& rhs);

/// Compute quotient of \p lhs and \p rhs
APFloat div(APFloat const& lhs, APFloat const& rhs);

/// Flip sign of \p operand
APFloat negate(APFloat operand);

/// Set precision of \p operand to \p precision
APFloat precisionCast(APFloat operand, APFloatPrec precision);

/// Compare \p lhs and \p rhs
int cmp(APFloat const& lhs, APFloat const& rhs);

/// Compare \p lhs and \p rhs
int cmp(APFloat const& lhs, double rhs);

/// ## Common math functions

/// Compute absolute value of \p arg
APFloat abs(APFloat const& arg);

/// Compute _e_ raised to the power of \p arg
APFloat exp(APFloat const& arg);

/// Compute 2 raised to the power of \p arg
APFloat exp2(APFloat const& arg);

/// Compute 10 raised to the power of \p arg
APFloat exp10(APFloat const& arg);

/// Compute the natural logarithm of \p arg
APFloat log(APFloat const& arg);

/// Compute the logarithm base 2 of \p arg
APFloat log2(APFloat const& arg);

/// Compute the logarithm base 10 of \p arg
APFloat log10(APFloat const& arg);

/// Compute \p base raised to the power of \p exp
APFloat pow(APFloat const& base, APFloat const& exp);

/// Compute square root of \p arg
APFloat sqrt(APFloat const& arg);

/// Compute cube root of \p arg
APFloat cbrt(APFloat const& arg);

/// Compute `hypot` of arguments
APFloat hypot(APFloat const& a, APFloat const& b);

/// Compute sine of \p arg
APFloat sin(APFloat const& arg);

/// Compute cosine of \p arg
APFloat cos(APFloat const& arg);

/// Compute tangent of \p arg
APFloat tan(APFloat const& arg);

/// Compute arc sine of \p arg
APFloat asin(APFloat const& arg);

/// Compute arc cosine of \p arg
APFloat acos(APFloat const& arg);

/// Compute arc tangent of \p arg
APFloat atan(APFloat const& arg);

/// ## class APFloat

/// Arbitraty precision floating point type.
/// Mantissa and exponent width is specified on construction and can be modified
/// with `setPrecision()` Operations involving multiple `APFloat`s usually
/// require the operands to be of same precision. \Note Right now this is
/// implemented in terms of hardware float operations on `float` or `double`
/// type. This is temporary until we take the time to implement proper IEEE
/// emulation. Until then only single and double precision are supported.
class APFloat {
    using Limb = std::uint64_t;

public:
    /// Construct an `APFloat` with `Double` precision
    APFloat(): APFloat(APFloatPrec::Double) {}
    
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

    /// Returns wether the value is negative.
    bool negative() const;

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
        std::string_view str, APFloatPrec precision = APFloatPrec::Double);

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
    bool isSingle() const { return precision() == APFloatPrec::Single; }

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
