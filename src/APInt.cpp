#include "APInt.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>
#include <tuple>

using namespace APMath;

using std::size_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

APInt::APInt(size_t bitwidth): APInt(0, bitwidth) {}

APInt::APInt(uint64_t value, size_t bw):
    _bitwidth(static_cast<uint32_t>(bw)),
    topLimbActiveBits(static_cast<uint32_t>(ceilRem(bitwidth(), limbSize * CHAR_BIT)))
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

APInt::APInt(std::span<Limb const> limbs, size_t bitwidth): APInt(bitwidth) {
    auto* const lp = limbPtr();
    std::copy_n(limbs.begin(), std::min(limbs.size(), numLimbs()), lp);
    lp[numLimbs() - 1] &= topLimbMask();
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
    size_t thisNumLimbs = numLimbs();
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
    size_t thisNumLimbs = numLimbs();
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
    Limb carry = 0;
    Limb* l = limbPtr();
    Limb const* r = rhs.limbPtr();
    size_t const count = std::min(numLimbs(), rhs.numLimbs());
    for (size_t i = 0; i < count; ++i) {
        Limb const newCarry = l[i] > limbMax - r[i] || l[i] > limbMax - (r[i] + carry);
        l[i] += r[i] + carry;
        carry = newCarry;
    }
    for (size_t i = count; carry != 0 && i < numLimbs(); ++i) {
        carry = l[i] == limbMax;
        l[i] += 1;
    }
    l[numLimbs() - 1] &= topLimbMask();
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
    constexpr size_t limbSize = sizeof(APInt::Limb);
    /// Only little endian implementation.
    uint8_t const* l = reinterpret_cast<uint8_t const*>(lhs);
    uint8_t const* r = reinterpret_cast<uint8_t const*>(rhs);
    for (size_t i = 0; i < limbSize; ++i) {
        auto const diff = static_cast<int>(l[i]) - static_cast<int>(r[i]);
        if (diff != 0) {
            return diff;
        }
    }
    return 0;
}

static int ucmpImpl(APInt::Limb const* lhs, size_t lhsNumLimbs, APInt::Limb const* rhs, size_t rhsNumLimbs) {
    if (lhsNumLimbs != rhsNumLimbs) {
        /// A one is bigger than the other, we need to test the top limbs if they are != 0
        auto const [bigPtr, bigSize, smallPtr, smallSize] = lhsNumLimbs > rhsNumLimbs ?
            std::tuple{ lhs, lhsNumLimbs, rhs, rhsNumLimbs } :
            std::tuple{ rhs, rhsNumLimbs, lhs, lhsNumLimbs };
        for (size_t i = smallSize; i != bigSize; ++i) {
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

int APInt::ucmp(uint64_t rhs) const {
    return ucmpImpl(limbPtr(), numLimbs(), &rhs, 1);
}

#include <sstream>

std::string APInt::toString(int base) const {
    // Hacky implementation for now only working for base 2
    assert(base == 2 && false);
//    std::stringstream sstr;
//    for (size_t i = numLimbs(); i > 0; --i) {
//        sstr <<
//    }
}

APInt::Limb* APInt::allocate(size_t numLimbs) {
    return static_cast<Limb*>(std::malloc(numLimbs * limbSize));
}

void APInt::deallocate(Limb* ptr, size_t numLimbs) {
    std::free(ptr);
}
