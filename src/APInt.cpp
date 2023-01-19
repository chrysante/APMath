#include "APInt.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <array>
#include <tuple>
#include <utility>

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

void APInt::swap(APInt& rhs) noexcept {
    std::swap(_bitwidth, rhs._bitwidth);
    std::swap(topLimbActiveBits, rhs.topLimbActiveBits);
    /// Use memcpy to swap to avoid reading inactive union member.
    Limb tmp;
    std::memcpy(&tmp, &singleLimb, limbSize);
    std::memcpy(&singleLimb, &rhs.singleLimb, limbSize);
    std::memcpy(&rhs.singleLimb, &tmp, limbSize);
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
    Limb carry = 0;
    Limb* l = limbPtr();
    Limb const* r = rhs.limbPtr();
    size_t const count = std::min(numLimbs(), rhs.numLimbs());
    for (size_t i = 0; i < count; ++i) {
        Limb const newCarry = l[i] < r[i] || l[i] < r[i] + carry;
        l[i] -= r[i] + carry;
        carry = newCarry;
    }
    for (size_t i = count; carry != 0 && i < numLimbs(); ++i) {
        carry = l[i] == 0;
        l[i] -= 1;
    }
    l[numLimbs() - 1] &= topLimbMask();
    return *this;
}

static APInt::Limb mulCarry(APInt::Limb a, APInt::Limb b) {
    static_assert(sizeof(APInt::Limb) == 8, "Only works for 64 bit integers");
    constexpr uint64_t mask = 0xFFFF'FFFF;
    uint64_t const a0 = a & mask;
    uint64_t const a1 = a >> 32;
    uint64_t const b0 = b & mask;
    uint64_t const b1 = b >> 32;
    uint64_t const d0 = a1 * b0 + (a0 * b0 >> 32);
    uint64_t const d1 = a0 * b1;
    uint64_t const c1 = d0 + d1;
    uint64_t const c2 = (c1 >> 32) + (c1 < d0 ? 0x100000000u : 0);
    uint64_t const carry = a1 * b1 + c2;
    return carry;
}

APInt& APInt::mul(APInt const& rhs) {
    assert(bitwidth() == rhs.bitwidth());
    APInt lhs(bitwidth());
    swap(lhs);
    Limb const* l = lhs.limbPtr();
    Limb const* r = rhs.limbPtr();
    APInt term(bitwidth());
    Limb* t = term.limbPtr();
    for (size_t j = numLimbs(); j > 0; ) {
        std::memset(t, 0, byteSize());
        --j;
        Limb const factor = r[j];
        Limb carry = 0;
        for (size_t i = 0, k = j; k < numLimbs(); ++i, ++k) {
            Limb newCarry = mulCarry(l[i], factor);
            assert(newCarry != limbMax);
            t[k] = l[i] * factor;
            newCarry += t[k] > limbMax - carry;
            t[k] += carry;
            carry = newCarry;
        }
        this->add(term);
    }
    return *this;
}

std::pair<APInt, APInt> APMath::udivmod(APInt const& numerator, APInt const& denominator) {
    using Limb = APInt::Limb;
    constexpr size_t limbBitSize = sizeof(Limb) * CHAR_BIT;
    assert(numerator.bitwidth() == denominator.bitwidth());
    assert(denominator.ucmp(0) != 0);
    APInt quotient(0, numerator.bitwidth());
    APInt remainder(0, numerator.bitwidth());
    Limb* const q = quotient.limbPtr();
    Limb* const r = remainder.limbPtr();
    Limb const* const n = numerator.limbPtr();
    for (size_t i = numerator.bitwidth(); i > 0;) {
        --i;
        remainder.lshl(1);
        r[0] |= (n[i / limbBitSize] >> (i % limbBitSize)) & Limb(1);
        if (remainder.ucmp(denominator) >= 0) {
            remainder.sub(denominator);
            q[i / limbBitSize] |= Limb(1) << (i % limbBitSize);
        }
    }
    return { std::move(quotient), std::move(remainder) };
}

std::pair<APInt, APInt> APMath::sdivmod(APInt const& numerator, APInt const& denominator) {
//    if (denominator.scmp(0) < 0) {
//        auto result = sdivmod(numerator, -denominator)
//    } then (Q, R) := divide(N, −D); return (−Q, R) end
//    
}

//function divide(N, D)
//  if D = 0 then error(DivisionByZero) end
//  if D < 0 then (Q, R) := divide(N, −D); return (−Q, R) end
//  if N < 0 then
//    (Q,R) := divide(−N, D)
//    if R = 0 then return (−Q, 0)
//    else return (−Q − 1, D − R) end
//  end
//  -- At this point, N ≥ 0 and D > 0
//  return divide_unsigned(N, D)
//end

APInt& APInt::sdiv(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::udiv(APInt const& rhs) {
    return *this = udivmod(*this, rhs).first;
}

APInt& APInt::srem(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::urem(APInt const& rhs) {
    return *this = udivmod(*this, rhs).second;
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

static void lshlShort(APInt::Limb* l, size_t numLimbs, size_t bitOffset) {
    using Limb = APInt::Limb;
    constexpr size_t limbBitSize = sizeof(Limb) * CHAR_BIT;
    assert(bitOffset < limbBitSize);
    Limb carry = 0;
    size_t const rightShiftAmount = limbBitSize - bitOffset;
    for (size_t i = 0; i < numLimbs; ++i) {
        Limb const newCarry = rightShiftAmount == limbBitSize ? 0 : l[i] >> rightShiftAmount;
        l[i] <<= bitOffset;
        l[i] |= carry;
        carry = newCarry;
    }
}

APInt& APInt::lshl(int numBits) {
    assert(numBits < _bitwidth);
    size_t const bitOffset = numBits % limbBitSize;
    size_t const limbOffset = numBits / limbBitSize;
    Limb* l = limbPtr();
    if (numBits < limbBitSize) {
        lshlShort(l, numLimbs(), bitOffset);
    }
    else {
        size_t i = numLimbs() - limbOffset;
        size_t j = numLimbs();
        while (i > 0) {
            --i;
            --j;
            l[j] = l[i];
        }
        while (j > 0) {
            --j;
            l[j] = 0;
        }
        lshlShort(l + limbOffset, numLimbs() - limbOffset, bitOffset);
    }
    return *this;
}

static void lshrShort(APInt::Limb* l, size_t numLimbs, size_t bitOffset) {
    using Limb = APInt::Limb;
    constexpr size_t limbBitSize = sizeof(Limb) * CHAR_BIT;
    assert(bitOffset < limbBitSize);
    Limb carry = 0;
    size_t const leftShiftAmount = limbBitSize - bitOffset;
    for (size_t i = numLimbs; i > 0;) {
        --i;
        Limb const newCarry = leftShiftAmount == limbBitSize ? 0 : l[i] << leftShiftAmount;
        l[i] >>= bitOffset;
        l[i] |= carry;
        carry = newCarry;
    }
}

APInt& APInt::lshr(int numBits) {
    assert(numBits < _bitwidth);
    size_t const bitOffset = numBits % limbBitSize;
    size_t const limbOffset = numBits / limbBitSize;
    Limb* l = limbPtr();
    if (numBits < limbBitSize) {
        lshrShort(l, numLimbs(), bitOffset);
    }
    else {
        size_t i = limbOffset;
        size_t j = 0;
        for (; i < numLimbs(); ++i, ++j) {
            l[j] = l[i];
        }
        for (; j < numLimbs(); ++j) {
            l[j] = 0;
        }
        lshrShort(l, numLimbs() - limbOffset, bitOffset);
    }
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
    assert(bitwidth() == rhs.bitwidth());
    int const l = static_cast<int>(limbPtr()[numLimbs() - 1] >> (topLimbActiveBits - 1));
    int const r = static_cast<int>(rhs.limbPtr()[numLimbs() - 1] >> (topLimbActiveBits - 1));
    if (l == r) {
        return ucmp(rhs);
    }
    else {
        return r - l;
    }
}

static int ucmpImpl(APInt::Limb const* lhs, size_t lhsNumLimbs, APInt::Limb const* rhs, size_t rhsNumLimbs) {
    if (lhsNumLimbs != rhsNumLimbs) {
        /// A one is bigger than the other, we need to test the top limbs separately.
        auto const [bigPtr, bigSize, smallPtr, smallSize] = lhsNumLimbs > rhsNumLimbs ?
            std::tuple{ lhs, lhsNumLimbs, rhs, rhsNumLimbs } :
            std::tuple{ rhs, rhsNumLimbs, lhs, lhsNumLimbs };
        for (size_t i = smallSize; i != bigSize; ++i) {
            if (bigPtr[i] != 0) {
                return bigPtr == lhs ? 1 : -1;
            }
        }
    }
    for (size_t i = std::min(lhsNumLimbs, rhsNumLimbs); i > 0;) {
        --i;
        if (lhs[i] == rhs[i]) {
            continue;
        }
        if (lhs[i] < rhs[i]) {
            return -1;
        }
        return 1;
    }
    return 0;
}

int APInt::ucmp(APInt const& rhs) const {
    return ucmpImpl(limbPtr(), numLimbs(), rhs.limbPtr(), rhs.numLimbs());
}

int APInt::ucmp(uint64_t rhs) const {
    return ucmpImpl(limbPtr(), numLimbs(), &rhs, 1);
}

std::string APInt::toString(int base) const {
    assert(false);
}

APInt::Limb* APInt::allocate(size_t numLimbs) {
    return static_cast<Limb*>(std::malloc(numLimbs * limbSize));
}

void APInt::deallocate(Limb* ptr, size_t numLimbs) {
    std::free(ptr);
}
