#ifndef APMATH_APFLOAT_H_
#define APMATH_APFLOAT_H_

#include <cstdint>
#include <cstddef>
#include <climits>
#include <compare>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <optional>
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

/// Arbitraty precision floating point type.
/// Mantissa and exponent width is specified on construction and can be modified with \p setPrecision()
/// Operations involving multiple APFloats usually require the operands to be of same precision.
/// \Note Right now this is implemented in terms of hardware float operations on \p float or \p double type. This is
/// temporary until we take the time to implement proper IEEE emulation. Until then only single and double precision are
/// supported. 
class APFloat {
public:
    /// Construct an \p APFloat with \p precision
    explicit APFloat(APFloatPrec precision);
    
    /// Construct an \p APFloat with \p precision and set it to \p value
    explicit APFloat(long double value, APFloatPrec precision);
    
    APFloat(APFloat const& rhs);
    APFloat(APFloat&& rhs) noexcept;
    APFloat& operator=(APFloat const& rhs);
    APFloat& operator=(APFloat&& rhs) noexcept;
    ~APFloat();
    
    /// Swap \p *this and \p rhs
    void swap(APFloat& rhs) noexcept;
    
    /// \code *this += rhs \endcode
    APFloat& add(APFloat const& rhs);
    
    /// \code *this -= rhs \endcode
    APFloat& sub(APFloat const& rhs);
    
    /// \code *this *= rhs \endcode
    APFloat& mul(APFloat const& rhs);
    
    /// \code *this /= rhs \endcode
    APFloat& div(APFloat const& rhs);
    
    /// \code *this = -*this \endcode
    APFloat& negate();
    
    /// Set precision to \p precision
    APFloat& setPrecision(APFloatPrec precision);

    /// Compare \p *this and \p rhs
    int cmp(APFloat const& rhs) const;
    
    /// Returns wether this float is negative.
    bool negative() const;
    
    /// The bitwidth of this integer.
    APFloatPrec precision() const { return { _mantWidth, _expWidth }; }
    
    /// Convert \p *this to a string in the specified base.
    std::string toString();
    
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
    
    /// Try to convert \p str to \p APFloat
    /// \param str All characters except ones representing digits in the specified base, an initial '-' and the first '.' are ignored.
    /// Result is exactly 64 bits wide. (For now)
    static std::optional<APFloat> parse(std::string_view str);
    
    /// Compare for equality.
    bool operator==(APFloat const& rhs) const { return cmp(rhs) == 0; }

    /// Relational comparison.
    std::strong_ordering operator<=>(APFloat const& rhs) const {
        int const res = cmp(rhs);
        if (res < 0) { return std::strong_ordering::less; }
        if (res > 0) { return std::strong_ordering::greater; }
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
    };
};

} // namespace APMath

#endif // APMATH_APFLOAT_H_
