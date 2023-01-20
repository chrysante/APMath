#include <cstdint>
#include <cstddef>
#include <climits>
#include <initializer_list>
#include <span>
#include <string>
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

APInt add(APInt lhs, APInt const& rhs);
APInt sub(APInt lhs, APInt const& rhs);
APInt mul(APInt const& lhs, APInt const& rhs);
std::pair<APInt, APInt> udivrem(APInt const& numerator, APInt const& divisor);
APInt udiv(APInt const& lhs, APInt const& rhs);
APInt urem(APInt const& lhs, APInt const& rhs);
std::pair<APInt, APInt> sdivrem(APInt const& numerator, APInt const& divisor);
APInt sdiv(APInt const& lhs, APInt const& rhs);
APInt srem(APInt const& lhs, APInt const& rhs);

APInt btwand(APInt lhs, APInt const& rhs);
APInt btwor(APInt lhs, APInt const& rhs);
APInt btwxor(APInt lhs, APInt const& rhs);

APInt lshl(APInt operand, int numBits);
APInt lshr(APInt operand, int numBits);

APInt ashl(APInt operand, int numBits);
APInt ashr(APInt operand, int numBits);

APInt rotl(APInt operand, int numBits);
APInt rotr(APInt operand, int numBits);

APInt negate(APInt operand);
APInt btwnot(APInt operand);

int scmp(APInt const& lhs, APInt const& rhs);
int ucmp(APInt const& lhs, APInt const& rhs);
int ucmp(APInt const& lhs, std::uint64_t rhs);

/// Arbitraty precision integer.
class APInt {
public:
    using Limb = internal::Limb;
    
public:
    explicit APInt(std::size_t bitwidth);
    explicit APInt(std::uint64_t value, std::size_t bitwidth);
    explicit APInt(std::initializer_list<Limb> limbs, std::size_t bitwidth):
        APInt(std::span<Limb const>(limbs), bitwidth) {}
    explicit APInt(std::span<Limb const> limbs, std::size_t bitwidth);
    APInt(APInt const& rhs);
    APInt(APInt&& rhs) noexcept;
    APInt& operator=(APInt const& rhs);
    APInt& operator=(APInt&& rhs) noexcept;
    ~APInt();
    
    void swap(APInt& rhs) noexcept;
    
    APInt& add(APInt const& rhs);
    APInt& sub(APInt const& rhs);
    APInt& mul(APInt const& rhs);
    APInt& udiv(APInt const& rhs);
    APInt& urem(APInt const& rhs);
    APInt& sdiv(APInt const& rhs);
    APInt& srem(APInt const& rhs);
    
    APInt& btwand(APInt const& rhs);
    APInt& btwor(APInt const& rhs);
    APInt& btwxor(APInt const& rhs);
    
    APInt& lshl(int numBits);
    APInt& lshr(int numBits);
    
    APInt& ashl(int numBits);
    APInt& ashr(int numBits);
    
    APInt& rotl(int numBits);
    APInt& rotr(int numBits);

    APInt& negate();
    APInt& btwnot();
    
    APInt& zext(std::size_t bitwidth);
    APInt& sext(std::size_t bitwidth);
    
    int scmp(APInt const& rhs) const;
    bool negative() const;
    int ucmp(APInt const& rhs) const;
    int ucmp(std::uint64_t rhs) const;
    
    std::size_t bitwidth() const { return _bitwidth; }
    
    static constexpr std::size_t maxBitwidth() { return (std::size_t(1) << 31) - 1; }
    
    std::string toString(int base = 10) const;
    
    friend APInt mul(APInt const& lhs, APInt const& rhs);
    friend std::pair<APInt, APInt> udivrem(APInt const& numerator, APInt const& divisor);
    friend std::pair<APInt, APInt> sdivrem(APInt const& numerator, APInt const& divisor);
    
private:
    bool isLocal() const { return numLimbs() <= 1; }
    
    std::size_t numLimbs() const {
        return internal::ceilDiv(_bitwidth, internal::limbBitSize);
    }
    
    std::size_t byteSize() const { return numLimbs() * internal::limbSize; }
    
    Limb topLimbMask() const { return topLimbActiveBits == 64 ? Limb(-1) : (Limb(1) << topLimbActiveBits) - 1; }
    
    Limb const* limbPtr() const { return isLocal() ? &singleLimb : limbs; }
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
        Limb* limbs;
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
