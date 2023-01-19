#include <cstdint>
#include <cstddef>

namespace APMath {

/// Arbitraty precision integer.
class APInt {
public:
    explicit APInt(std::size_t precision);
    explicit APInt(std::uint64_t value, std::size_t precision);
    APInt(APInt const&);
    APInt(APInt&&) noexcept;
    APInt& operator=(APInt const&);
    APInt& operator=(APInt&&) noexcept;
    ~APInt();
    
    APInt& add(APInt const& rhs);
    APInt& sub(APInt const& rhs);
    APInt& mul(APInt const& rhs);
    APInt& sdiv(APInt const& rhs);
    APInt& udiv(APInt const& rhs);
    APInt& srem(APInt const& rhs);
    APInt& urem(APInt const& rhs);
    
    APInt& btwand(APInt const& rhs);
    APInt& btwor(APInt const& rhs);
    APInt& btwxor(APInt const& rhs);
    
    APInt& btwnot();
    
    APInt& lshl(int numBits);
    APInt& lshr(int numBits);
    
    APInt& ashl(int numBits);
    APInt& ashr(int numBits);
    
    APInt& rotl(int numBits);
    APInt& rotr(int numBits);
    
    int scmp(APInt const& rhs) const;
    int scmp(std::int64_t rhs) const;
    int ucmp(APInt const& rhs) const;
    int ucmp(std::uint64_t rhs) const;
    
    std::size_t bitwidth() const { return _bitwidth; }
    
    static constexpr std::size_t maxBitwidth() { return (std::size_t(1) << 31) - 1; }
    
private:
    union {
        std::uint64_t singleLimb;
        std::uint64_t* limbs;
    };
    bool isLocal            :  1;
    std::uint32_t _bitwidth  : 31;
    std::uint32_t _numLimbs;
};

} // namespace APMath
