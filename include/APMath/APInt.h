#ifndef APMATH_APINT_H_
#define APMATH_APINT_H_

#include <cstdint>
#include <cstddef>
#include <climits>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <optional>
#include <utility>

namespace APMath::internal {

using Limb = std::uint64_t;

static constexpr std::size_t limbSize = sizeof(Limb);
static constexpr std::size_t limbBitSize = limbSize * CHAR_BIT;
static constexpr Limb limbMax = Limb(-1);

std::size_t ceilDiv(std::size_t a, std::size_t b);
std::size_t ceilRem(std::size_t a, std::size_t b);

} // namespace APMath::internal

namespace APMath {

class APInt;

/// Compute sum of \p lhs and \p rhs
APInt add(APInt lhs, APInt const& rhs);

/// Compute difference of \p lhs and \p rhs
APInt sub(APInt lhs, APInt const& rhs);

/// Compute product of \p lhs and \p rhs
APInt mul(APInt const& lhs, APInt const& rhs);

/// Compute quotient and remainder of \p lhs and \p rhs
/// Operands are interpreted as unsigned integers.
std::pair<APInt, APInt> udivrem(APInt const& numerator, APInt const& divisor);

/// Compute quotient of \p lhs and \p rhs
/// Operands are interpreted as unsigned integers.
APInt udiv(APInt const& lhs, APInt const& rhs);

/// Compute remainder of \p lhs and \p rhs
/// Operands are interpreted as unsigned integers.
APInt urem(APInt const& lhs, APInt const& rhs);

/// Compute quotient and remainder of \p lhs and \p rhs
/// Operands are interpreted as signed integers. Quotient is truncated towards 0.
std::pair<APInt, APInt> sdivrem(APInt const& numerator, APInt const& divisor);

/// Compute quotient of \p lhs and \p rhs
/// Operands are interpreted as signed integers. Result is truncated towards 0.
APInt sdiv(APInt const& lhs, APInt const& rhs);

/// Compute remainder of \p lhs and \p rhs
/// Operands are interpreted as signed integers.
APInt srem(APInt const& lhs, APInt const& rhs);

/// Compute bitwise AND of \p lhs and \p rhs
APInt btwand(APInt lhs, APInt const& rhs);

/// Compute bitwise OR of \p lhs and \p rhs
APInt btwor(APInt lhs, APInt const& rhs);

/// Compute bitwise XOR of \p lhs and \p rhs
APInt btwxor(APInt lhs, APInt const& rhs);

/// Logical left shift \p operand by \p numBits bits.
APInt lshl(APInt operand, int numBits);

/// Logical right shift \p operand by \p numBits bits.
APInt lshr(APInt operand, int numBits);

/// Arithmetic left shift \p operand by \p numBits bits.
APInt ashl(APInt operand, int numBits);

/// Arithmetic right shift \p operand by \p numBits bits.
APInt ashr(APInt operand, int numBits);

/// Left rotate \p operand by \p numBits bits.
APInt rotl(APInt operand, int numBits);

/// Right rotate \p operand by \p numBits bits.
APInt rotr(APInt operand, int numBits);

/// Compute arithmetic signed complement of \p operand
APInt negate(APInt operand);

/// Compute bitwise comlement of \p operand
APInt btwnot(APInt operand);

/// Zero-extend \p operand to \p bitwidth
/// If \p bitwidth is less than current bitwidth, \p operand will be shrunk.
APInt zext(APInt operand, std::size_t bitwidth);

/// Sign-extend \p operand to \p bitwidth
/// If \p bitwidth is less than current bitwidth, \p operand will be shrunk.
APInt sext(APInt operand, std::size_t bitwidth);

/// Perform unsigned comparison between \p lhs and \p rhs
int ucmp(APInt const& lhs, APInt const& rhs);

/// \overload
int ucmp(APInt const& lhs, std::uint64_t rhs);

/// \overload
int ucmp(std::uint64_t lhs, APInt const& rhs);

/// Perform signed comparison between \p lhs and \p rhs
int scmp(APInt const& lhs, APInt const& rhs);

/// Arbitraty width integer.
/// Bit width is specified on construction and can be modified with \p zext() and \p sext()
/// Operations involving multiple integers usually require the operands to be of same width.
class APInt {
public:
    using Limb = internal::Limb;
    
public:
    /// Construct an \p APInt with \p bitwidth
    explicit APInt(std::size_t bitwidth);
    
    /// Construct an \p APInt with \p bitwidth and set it to \p value
    explicit APInt(std::uint64_t value, std::size_t bitwidth);
    
    /// Construct an \p APInt with \p bitwidth and set its limbs to \p limbs
    explicit APInt(std::initializer_list<Limb> limbs, std::size_t bitwidth):
        APInt(std::span<Limb const>(limbs), bitwidth) {}
    
    /// Construct an \p APInt with \p bitwidth and set its limbs to \p limbs
    explicit APInt(std::span<Limb const> limbs, std::size_t bitwidth);
    
    APInt(APInt const& rhs);
    APInt(APInt&& rhs) noexcept;
    APInt& operator=(APInt const& rhs);
    APInt& operator=(APInt&& rhs) noexcept;
    ~APInt();
    
    /// Swap \p *this and \p rhs
    void swap(APInt& rhs) noexcept;
    
    /// \code *this += rhs \endcode
    APInt& add(APInt const& rhs);
    
    /// \code *this -= rhs \endcode
    APInt& sub(APInt const& rhs);
    
    /// \code *this *= rhs \endcode
    APInt& mul(APInt const& rhs);
    
    /// \code *this /= rhs \endcode
    /// Both operands are interpreted as unsigned integers
    APInt& udiv(APInt const& rhs);
    
    /// \code *this %= rhs \endcode
    /// Both operands are interpreted as unsigned integers
    APInt& urem(APInt const& rhs);
    
    /// \code *this /= rhs \endcode
    /// Both operands are interpreted as signed integers
    APInt& sdiv(APInt const& rhs);
    
    /// \code *this %= rhs \endcode
    /// Both operands are interpreted as signed integers
    APInt& srem(APInt const& rhs);
    
    /// \code *this &= rhs \endcode
    APInt& btwand(APInt const& rhs);
    
    /// \code *this |= rhs \endcode
    APInt& btwor(APInt const& rhs);
    
    /// \code *this ^= rhs \endcode
    APInt& btwxor(APInt const& rhs);
    
    /// Logical left shift \p *this by \p numBits bits.
    APInt& lshl(int numBits);
    
    /// Logical right shift \p *this by \p numBits bits.
    APInt& lshr(int numBits);
    
    /// Arithmetic left shift \p *this by \p numBits bits.
    APInt& ashl(int numBits);
    
    /// Arithmetic right shift \p *this by \p numBits bits.
    APInt& ashr(int numBits);
    
    /// Left rotate \p *this by \p numBits bits.
    APInt& rotl(int numBits);
    
    /// Right rotate \p *this by \p numBits bits.
    APInt& rotr(int numBits);

    /// Copute and assign arithmetic signed complement of \p *this
    APInt& negate();
    
    /// Copute and assign bitwise complement of \p *this
    APInt& btwnot();
    
    /// Perform zero extend to \p bitwidth
    /// If \p bitwidth is less than current bitwidth, \p *this will be shrunk.
    APInt& zext(std::size_t bitwidth);
    
    /// Perform sign extend to \p bitwidth
    /// If \p bitwidth is less than current bitwidth, \p *this will be shrunk.
    APInt& sext(std::size_t bitwidth);
    
    /// Perform unsigned comparison between \p *this and \p rhs
    int ucmp(APInt const& rhs) const;

    /// \overload
    int ucmp(std::uint64_t rhs) const;
    
    /// Perform signed comparison between \p *this and \p rhs
    int scmp(APInt const& rhs) const;
    
    /// Returns wether this integer is negative.
    bool negative() const;
    
    /// The bitwidth of this integer.
    std::size_t bitwidth() const { return _bitwidth; }
    
    /// The maximum number of bits any \p APInt can hold.
    static constexpr std::size_t maxBitwidth() { return std::numeric_limits<std::uint32_t>::max(); }
    
    /// Convert \p *this to a string in the specified base.
    /// \p this is interpreted as an unsigned integer.
    /// \param base must be between 0 and 36 (inclusive)
    std::string toString(int base = 10) const&;
    
    /// \overload
    std::string toString(int base = 10)&&;
    
    /// Convert \p *this to a string in the specified base.
    /// \p this is interpreted as a signed integer.
    /// \param base must be between 0 and 36 (inclusive)
    std::string signedToString(int base = 10) const;
    
    /// View over limbs
    std::span<Limb const> limbs() const { return { limbPtr(), numLimbs() }; }
    
    /// Convert to native integral type
    template <typename T>
    T to() const {
        static_assert(std::is_arithmetic_v<T>);
        assert(bitwidth() <= sizeof(T) * CHAR_BIT);
        return static_cast<T>(limbPtr()[0]);
    }
    
    /// Compute a 64 bit of of this integer.
    /// Note that this is not a cryptographic hash.
    std::size_t hash() const;
    
    /// Try to convert \p str to \p APInt
    /// \param str All characters except ones representing digits in the specified base and an initial '-' are ignored.
    /// Result is exactly as wide as necessary to represent the parsed integer.
    /// \param base must be between 0 and 36 (inclusive)
    static std::optional<APInt> parse(std::string_view str, int base = 10);
    
    /// Compare integers for equality.
    /// Note that relational comparisons are not exposed as C++ operators due to the ambiguity of signedness.
    /// Use \p ucmp() and \p scmp() to compare order.
    bool operator==(APInt const& rhs) const { return ucmp(rhs) == 0; }
    
    /// \overload
    bool operator==(std::uint64_t rhs) const { return ucmp(rhs) == 0; }
    
private:
    friend APInt mul(APInt const& lhs, APInt const& rhs);
    friend std::pair<APInt, APInt> udivrem(APInt const& numerator, APInt const& divisor);
    friend std::pair<APInt, APInt> sdivrem(APInt const& numerator, APInt const& divisor);
    
    bool isLocal() const { return numLimbs() <= 1; }
    
    std::size_t numLimbs() const {
        return internal::ceilDiv(_bitwidth, internal::limbBitSize);
    }
    
    std::size_t byteSize() const { return numLimbs() * internal::limbSize; }
    
    Limb topLimbMask() const { return topLimbActiveBits == 64 ? Limb(-1) : (Limb(1) << topLimbActiveBits) - 1; }
    
    Limb const* limbPtr() const { return isLocal() ? &singleLimb : heapLimbs; }
    Limb* limbPtr() { return const_cast<Limb*>(static_cast<APInt const*>(this)->limbPtr()); }
    
    int highbit() const {
        return static_cast<int>(limbPtr()[numLimbs() - 1] >> (topLimbActiveBits - 1));
    }
    
    Limb* allocate(std::size_t numLimbs);
    void deallocate(Limb* ptr, std::size_t numLimbs);
    
private:
    std::uint32_t _bitwidth;
    std::uint32_t topLimbActiveBits;
    union {
        Limb singleLimb;
        Limb* heapLimbs;
    };
};

} // namespace APMath

inline std::size_t APMath::internal::ceilDiv(std::size_t a, std::size_t b) {
    return a / b + !!(a % b);
}

inline std::size_t APMath::internal::ceilRem(std::size_t a, std::size_t b) {
    std::size_t const result = a % b;
    if (result == 0) {
        return b;
    }
    return result;
}

template <>
struct std::hash<APMath::APInt> {
    std::size_t operator()(APMath::APInt const& value) const {
        return value.hash();
    }
};

#endif // APMATH_APINT_H_
