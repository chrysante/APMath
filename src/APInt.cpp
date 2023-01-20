#include "APInt.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <array>
#include <tuple>
#include <utility>

using namespace APMath;
using namespace APMath::internal;

using std::size_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

APInt APMath::add(APInt lhs, APInt const& rhs) {
    return lhs.add(rhs);
}

APInt APMath::sub(APInt lhs, APInt const& rhs) {
    return lhs.sub(rhs);
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

APInt APMath::mul(APInt const& lhs, APInt const& rhs) {
    assert(lhs.bitwidth() == rhs.bitwidth());
    Limb const* l = lhs.limbPtr();
    Limb const* r = rhs.limbPtr();
    APInt term(lhs.bitwidth());
    Limb* t = term.limbPtr();
    APInt res(lhs.bitwidth());
    for (size_t j = lhs.numLimbs(); j > 0; ) {
        std::memset(t, 0, lhs.byteSize());
        --j;
        Limb const factor = r[j];
        Limb carry = 0;
        for (size_t i = 0, k = j; k < lhs.numLimbs(); ++i, ++k) {
            Limb newCarry = mulCarry(l[i], factor);
            assert(newCarry != limbMax);
            t[k] = l[i] * factor;
            newCarry += t[k] > limbMax - carry;
            t[k] += carry;
            carry = newCarry;
        }
        res.add(term);
    }
    return res;
}

std::pair<APInt, APInt> APMath::udivrem(APInt const& numerator, APInt const& denominator) {
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

APInt APMath::udiv(APInt const& lhs, APInt const& rhs) {
    return udivrem(lhs, rhs).first;
}

APInt APMath::urem(APInt const& lhs, APInt const& rhs) {
    return udivrem(lhs, rhs).second;
}

std::pair<APInt, APInt> APMath::sdivrem(APInt const& numerator, APInt const& denominator) {
    if (denominator.negative()) {
        auto [q, r] = sdivrem(numerator, negate(denominator));
        return { std::move(q.negate()), std::move(r) };
    }
    if (numerator.negative()) {
        auto [q, r] = udivrem(negate(numerator), denominator);
        return { std::move(q.negate()), std::move(r.negate()) };
    }
    return udivrem(numerator, denominator);
}

APInt APMath::sdiv(APInt const& lhs, APInt const& rhs) {
    return sdivrem(lhs, rhs).first;
}

APInt APMath::srem(APInt const& lhs, APInt const& rhs) {
    return sdivrem(lhs, rhs).second;
}

APInt APMath::btwand(APInt lhs, APInt const& rhs) {
    return lhs.btwand(rhs);
}

APInt APMath::btwor(APInt lhs, APInt const& rhs) {
    return lhs.btwor(rhs);
}

APInt APMath::btwxor(APInt lhs, APInt const& rhs) {
    return lhs.btwxor(rhs);
}

APInt APMath::lshl(APInt operand, int numBits) {
    return operand.lshl(numBits);
}

APInt APMath::lshr(APInt operand, int numBits) {
    return operand.lshr(numBits);
}

APInt APMath::ashl(APInt operand, int numBits) {
    return operand.ashl(numBits);
}

APInt APMath::ashr(APInt operand, int numBits) {
    return operand.ashr(numBits);
}

APInt APMath::rotl(APInt operand, int numBits) {
    return operand.rotl(numBits);
}

APInt APMath::rotr(APInt operand, int numBits) {
    return operand.rotr(numBits);
}

APInt APMath::negate(APInt operand) {
    return operand.negate();
}

APInt APMath::btwnot(APInt operand) {
    return operand.btwnot();
}

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

APInt& APInt::mul(APInt const& rhs) {
    return *this = APMath::mul(*this, rhs);
}

APInt& APInt::udiv(APInt const& rhs) {
    return *this = APMath::udiv(*this, rhs);
}

APInt& APInt::urem(APInt const& rhs) {
    return *this = APMath::urem(*this, rhs);
}

APInt& APInt::sdiv(APInt const& rhs) {
    return *this = APMath::sdiv(*this, rhs);
}

APInt& APInt::srem(APInt const& rhs) {
    return *this = APMath::srem(*this, rhs);
}

APInt& APInt::btwand(APInt const& rhs) {
    assert(bitwidth() == rhs.bitwidth());
    Limb* const l = limbPtr();
    Limb const* const r = rhs.limbPtr();
    for (size_t i = 0; i < byteSize(); ++i) {
        l[i] &= r[i];
    }
    return *this;
}

APInt& APInt::btwor(APInt const& rhs) {
    assert(bitwidth() == rhs.bitwidth());
    Limb* const l = limbPtr();
    Limb const* const r = rhs.limbPtr();
    for (size_t i = 0; i < byteSize(); ++i) {
        l[i] |= r[i];
    }
    return *this;
}

APInt& APInt::btwxor(APInt const& rhs) {
    assert(bitwidth() == rhs.bitwidth());
    Limb* const l = limbPtr();
    Limb const* const r = rhs.limbPtr();
    for (size_t i = 0; i < byteSize(); ++i) {
        l[i] ^= r[i];
    }
    l[numLimbs() - 1] &= topLimbMask();
    return *this;
}

static void lshlShort(APInt::Limb* l, size_t numLimbs, size_t bitOffset) {
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
    assert(false);
    return *this;
}

APInt& APInt::ashr(int numBits) {
    assert(false);
    return *this;
}

APInt& APInt::rotl(int numBits) {
    assert(false);
    return *this;
}

APInt& APInt::rotr(int numBits) {
    assert(false);
    return *this;
}

APInt& APInt::negate() {
    Limb carry = 0;
    Limb* l = limbPtr();
    for (size_t i = 0; i < numLimbs(); ++i) {
        Limb const newCarry = l[i] > 0 || l[i] + carry > 0;
        l[i] = -l[i] - carry;
        carry = newCarry;
    }
    l[numLimbs() - 1] &= topLimbMask();
    return *this;
}

APInt& APInt::btwnot() {
    Limb* const l = limbPtr();
    for (size_t i = 0; i < byteSize(); ++i) {
        l[i] = ~l[i];
    }
    l[numLimbs() - 1] &= topLimbMask();
    return *this;
}

APInt& APInt::zext(size_t bitwidth) {
    return *this = APInt({ limbPtr(), numLimbs() }, bitwidth);
}

APInt& APInt::sext(size_t bitwidth) {
    int const h = highbit();
    size_t const oldWidth = this->bitwidth();
    Limb const oldTopMask = topLimbMask();
    size_t const oldnumLimbs = numLimbs();
    zext(bitwidth);
    if (oldWidth >= bitwidth || h == 0) {
        return *this;
    }
    Limb* const l = limbPtr();
    l[oldnumLimbs - 1] |= ~oldTopMask;
    for (size_t i = oldnumLimbs; i < numLimbs(); ++i) {
        l[i] = Limb(-1);
    }
    l[numLimbs() - 1] &= topLimbMask();
    return *this;
}

int APInt::scmp(APInt const& rhs) const {
    assert(bitwidth() == rhs.bitwidth());
    int const l = highbit();
    int const r = rhs.highbit();
    if (l == r) {
        return ucmp(rhs);
    }
    else {
        return r - l;
    }
}

bool APInt::negative() const {
    return highbit() != 0;
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
