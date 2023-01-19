#include "APInt.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>
#include <tuple>

using namespace APMath;

APInt::APInt(std::size_t bitwidth): APInt(0, bitwidth) {}

APInt::APInt(std::uint64_t value, std::size_t bw):
    _bitwidth(static_cast<std::uint32_t>(bw)),
    topLimbActiveBits(static_cast<std::uint32_t>(ceilRem(bitwidth(), limbSize * CHAR_BIT)))
{
    assert(bw <= maxBitwidth());
    if (isLocal()) {
        singleLimb = value & topLimbMask();
    }
    else {
        limbs = allocate(numLimbs());
        std::memset(limbs, 0, byteSize());
        limbs[0] = value;
    }
}

APInt::APInt(APInt const& rhs):
    _bitwidth(rhs._bitwidth),
    topLimbActiveBits(rhs.topLimbActiveBits)
{
    if (isLocal()) {
        singleLimb = rhs.singleLimb;
    }
    else {
        limbs = static_cast<Limb*>(std::malloc(byteSize()));
        std::memcpy(limbs, rhs.limbs, byteSize());
    }
}

APInt::APInt(APInt&& rhs) noexcept:
    _bitwidth(rhs._bitwidth),
    topLimbActiveBits(rhs.topLimbActiveBits)
{
    if (isLocal()) {
        singleLimb = rhs.singleLimb;
    }
    else {
        limbs = rhs.limbs;
        rhs._bitwidth = 0;
        rhs.topLimbActiveBits = 0;
        rhs.limbs = nullptr;
    }
}

APInt& APInt::operator=(APInt const& rhs) {
    bool thisIsLocal = isLocal();
    std::size_t thisNumLimbs = numLimbs();
    _bitwidth = rhs._bitwidth;
    topLimbActiveBits = rhs.topLimbActiveBits;
    if (thisIsLocal) {
        if (rhs.isLocal()) {
            singleLimb = rhs.singleLimb;
        }
        else {
            limbs = allocate(rhs.numLimbs());
            std::memcpy(limbs, rhs.limbs, rhs.byteSize());
        }
    }
    else {
        if (rhs.isLocal()) {
            deallocate(limbs, thisNumLimbs);
            singleLimb = rhs.singleLimb;
        }
        else {
            if (thisNumLimbs != rhs.numLimbs()) {
                /// Need to reallocate
                deallocate(limbs, thisNumLimbs);
                limbs = allocate(rhs.numLimbs());
            }
            std::memcpy(limbs, rhs.limbs, rhs.byteSize());
        }
    }
    return *this;
}

APInt& APInt::operator=(APInt&& rhs) noexcept {
    bool thisIsLocal = isLocal();
    std::size_t thisNumLimbs = numLimbs();
    _bitwidth = rhs._bitwidth;
    topLimbActiveBits = rhs.topLimbActiveBits;
    if (thisIsLocal) {
        if (rhs.isLocal()) {
            singleLimb = rhs.singleLimb;
        }
        else {
            /// We steal rhs's buffer
            limbs = rhs.limbs;
            rhs._bitwidth = 0;
            rhs.topLimbActiveBits = 0;
            rhs.limbs = nullptr;
        }
    }
    else {
        if (rhs.isLocal()) {
            deallocate(limbs, thisNumLimbs);
            singleLimb = rhs.singleLimb;
        }
        else {
            /// Both not local, we swap
            std::swap(_bitwidth, rhs._bitwidth);
            std::swap(topLimbActiveBits, rhs.topLimbActiveBits);
            std::swap(limbs, rhs.limbs);
        }
    }
    return *this;
}

APInt::~APInt() {
    if (isLocal()) {
        return;
    }
    deallocate(limbs, numLimbs());
}

APInt& APInt::add(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::sub(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::mul(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::sdiv(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::udiv(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::srem(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::urem(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::btwand(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::btwor(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::btwxor(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::btwnot() {
    
    return *this;
}

APInt& APInt::lshl(int numBits) {
    
    return *this;
}

APInt& APInt::lshr(int numBits) {
    
    return *this;
}

APInt& APInt::ashl(int numBits) {
    
    return *this;
}

APInt& APInt::ashr(int numBits) {
    
    return *this;
}

APInt& APInt::rotl(int numBits) {
    
    return *this;
}

APInt& APInt::rotr(int numBits) {
    
    return *this;
}

int APInt::scmp(APInt const& rhs) const {
    
    return 0;
}

int APInt::scmp(std::int64_t rhs) const {
    
    return 0;
}

static int ucmpLimb(APInt::Limb const* lhs, APInt::Limb const* rhs) {
    constexpr std::size_t limbSize = sizeof(APInt::Limb);
    /// Only little endian implementation.
    std::uint8_t const* l = reinterpret_cast<std::uint8_t const*>(lhs);
    std::uint8_t const* r = reinterpret_cast<std::uint8_t const*>(rhs);
    for (std::size_t i = 0; i < limbSize; ++i) {
        auto const diff = static_cast<int>(l[i]) - static_cast<int>(r[i]);
        if (diff != 0) {
            return diff;
        }
    }
    return 0;
}

static int ucmpImpl(APInt::Limb const* lhs, std::size_t lhsNumLimbs, APInt::Limb const* rhs, std::size_t rhsNumLimbs) {
    if (lhsNumLimbs != rhsNumLimbs) {
        /// A one is bigger than the other, we need to test the top limbs if they are != 0
        auto const [bigPtr, bigSize, smallPtr, smallSize] = lhsNumLimbs > rhsNumLimbs ?
            std::tuple{ lhs, lhsNumLimbs, rhs, rhsNumLimbs } :
            std::tuple{ rhs, rhsNumLimbs, lhs, lhsNumLimbs };
        for (std::size_t i = smallSize; i != bigSize; ++i) {
            if (bigPtr[i] != 0) {
                return bigPtr == lhs ? 1 : -1;
            }
        }
    }
    for (auto* end = lhs + std::min(lhsNumLimbs, rhsNumLimbs); lhs != end; ++lhs, ++rhs) {
        if (auto const diff = ucmpLimb(lhs, rhs); diff != 0) {
            return diff;
        }
    }
    return 0;
}

int APInt::ucmp(APInt const& rhs) const {
    return ucmpImpl(limbPtr(), numLimbs(), rhs.limbPtr(), rhs.numLimbs());
}

int APInt::ucmp(std::uint64_t rhs) const {
    return ucmpImpl(limbPtr(), numLimbs(), &rhs, 1);
}

APInt::Limb* APInt::allocate(std::size_t numLimbs) {
    return static_cast<Limb*>(std::malloc(numLimbs * limbSize));
}

void APInt::deallocate(Limb* ptr, std::size_t numLimbs) {
    std::free(ptr);
}
