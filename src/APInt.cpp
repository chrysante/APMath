#include <APMath/APInt.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <array>
#include <tuple>
#include <utility>
#include <vector>

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

std::pair<APInt, APInt> APMath::udivrem(APInt const& numerator,
                                        APInt const& denominator) {
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

std::pair<APInt, APInt> APMath::sdivrem(APInt const& numerator,
                                        APInt const& denominator) {
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

APInt APMath::zext(APInt operand, std::size_t bitwidth) {
    return operand.zext(bitwidth);
}

APInt APMath::sext(APInt operand, std::size_t bitwidth) {
    return operand.sext(bitwidth);
}

int APMath::ucmp(APInt const& lhs, APInt const& rhs) {
    return lhs.ucmp(rhs);
}

int APMath::ucmp(APInt const& lhs, std::uint64_t rhs) {
    return lhs.ucmp(rhs);
}

int APMath::ucmp(std::uint64_t lhs, APInt const& rhs) {
    return -rhs.ucmp(lhs);
}

int APMath::scmp(APInt const& lhs, APInt const& rhs) {
    return lhs.scmp(rhs);
}

APInt::APInt(size_t bitwidth): APInt(0, bitwidth) {}

APInt::APInt(uint64_t value, size_t bw):
    _bitwidth(static_cast<uint32_t>(bw)),
    topLimbActiveBits(static_cast<uint32_t>(ceilRem(bitwidth(),
                                                    limbSize * CHAR_BIT)))
{
    assert(bw > 0);
    assert(bw <= maxBitwidth());
    if (isLocal()) {
        singleLimb = value & topLimbMask();
    }
    else {
        heapLimbs = allocate(numLimbs());
        std::memset(heapLimbs, 0, byteSize());
        heapLimbs[0] = value;
    }
}

APInt::APInt(std::span<Limb const> limbs, size_t bitwidth): APInt(bitwidth) {
    assert(bitwidth > 0);
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
        heapLimbs = static_cast<Limb*>(std::malloc(byteSize()));
        std::memcpy(heapLimbs, rhs.heapLimbs, byteSize());
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
        heapLimbs = rhs.heapLimbs;
        rhs._bitwidth = 0;
        rhs.topLimbActiveBits = 0;
        rhs.heapLimbs = nullptr;
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
            heapLimbs = allocate(rhs.numLimbs());
            std::memcpy(heapLimbs, rhs.heapLimbs, rhs.byteSize());
        }
    }
    else {
        if (rhs.isLocal()) {
            deallocate(heapLimbs, thisNumLimbs);
            singleLimb = rhs.singleLimb;
        }
        else {
            if (thisNumLimbs != rhs.numLimbs()) {
                /// Need to reallocate
                deallocate(heapLimbs, thisNumLimbs);
                heapLimbs = allocate(rhs.numLimbs());
            }
            std::memcpy(heapLimbs, rhs.heapLimbs, rhs.byteSize());
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
            heapLimbs = rhs.heapLimbs;
            rhs._bitwidth = 0;
            rhs.topLimbActiveBits = 0;
            rhs.heapLimbs = nullptr;
        }
    }
    else {
        if (rhs.isLocal()) {
            deallocate(heapLimbs, thisNumLimbs);
            singleLimb = rhs.singleLimb;
        }
        else {
            /// Both not local, we swap
            std::swap(_bitwidth, rhs._bitwidth);
            std::swap(topLimbActiveBits, rhs.topLimbActiveBits);
            std::swap(heapLimbs, rhs.heapLimbs);
        }
    }
    return *this;
}

APInt::~APInt() {
    if (isLocal()) {
        return;
    }
    deallocate(heapLimbs, numLimbs());
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
    assert(bitwidth() == rhs.bitwidth());
    Limb carry = 0;
    Limb* l = limbPtr();
    Limb const* r = rhs.limbPtr();
    for (size_t i = 0; i < numLimbs(); ++i) {
        Limb const newCarry =
            l[i] > limbMax - r[i] || l[i] > limbMax - (r[i] + carry);
        l[i] += r[i] + carry;
        carry = newCarry;
    }
    l[numLimbs() - 1] &= topLimbMask();
    return *this;
}

APInt& APInt::sub(APInt const& rhs) {
    Limb carry = 0;
    Limb* l = limbPtr();
    Limb const* r = rhs.limbPtr();
    for (size_t i = 0; i < numLimbs(); ++i) {
        Limb const newCarry = l[i] < r[i] || l[i] < r[i] + carry;
        l[i] -= r[i] + carry;
        carry = newCarry;
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
    for (size_t i = 0; i < numLimbs(); ++i) {
        l[i] &= r[i];
    }
    return *this;
}

APInt& APInt::btwor(APInt const& rhs) {
    assert(bitwidth() == rhs.bitwidth());
    Limb* const l = limbPtr();
    Limb const* const r = rhs.limbPtr();
    for (size_t i = 0; i < numLimbs(); ++i) {
        l[i] |= r[i];
    }
    return *this;
}

APInt& APInt::btwxor(APInt const& rhs) {
    assert(bitwidth() == rhs.bitwidth());
    Limb* const l = limbPtr();
    Limb const* const r = rhs.limbPtr();
    for (size_t i = 0; i < numLimbs(); ++i) {
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
        Limb const newCarry =
            rightShiftAmount == limbBitSize ? 0 : l[i] >> rightShiftAmount;
        l[i] <<= bitOffset;
        l[i] |= carry;
        carry = newCarry;
    }
}

APInt& APInt::lshl(int nb) {
    assert(nb >= 0);
    assert(nb < _bitwidth);
    size_t const numBits = static_cast<size_t>(nb);
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
        Limb const newCarry =
            leftShiftAmount == limbBitSize ? 0 : l[i] << leftShiftAmount;
        l[i] >>= bitOffset;
        l[i] |= carry;
        carry = newCarry;
    }
}

APInt& APInt::lshr(int nb) {
    assert(nb >= 0);
    assert(nb < _bitwidth);
    size_t const numBits = static_cast<size_t>(nb);
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
    return lshl(numBits);
}

APInt& APInt::ashr(int nb) {
    assert(nb >= 0);
    size_t const numBits = static_cast<size_t>(nb);
    assert(numBits < _bitwidth);
    int const h = highbit();
    lshr(nb);
    if (h == 0) {
        return *this;
    }
    Limb* const l = limbPtr();
    size_t const loopEnd = ceilDiv(bitwidth() - numBits, limbBitSize);
    for (size_t i = numLimbs(); i > loopEnd; ) {
        --i;
        l[i] = Limb(-1);
    }
    if (topLimbActiveBits < numBits) {
        l[loopEnd - 1] |=
            Limb(-1) << (limbBitSize - (numBits - topLimbActiveBits));
    }
    else if (topLimbActiveBits == numBits) {
        l[loopEnd - 1] |= Limb(-1);
    }
    else {
        l[loopEnd - 1] |= Limb(-1) << (topLimbActiveBits - numBits);
    }
    l[numLimbs() - 1] &= topLimbMask();
    return *this;
}

APInt& APInt::rotl(int numBits) {
    assert(false);
    std::abort();
}

APInt& APInt::rotr(int numBits) {
    assert(false);
    std::abort();
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
    for (size_t i = 0; i < numLimbs(); ++i) {
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

static int ucmpImpl(APInt::Limb const* lhs,
                    size_t lhsNumLimbs,
                    APInt::Limb const* rhs,
                    size_t rhsNumLimbs) {
    if (lhsNumLimbs != rhsNumLimbs) {
        /// If one is bigger than the other, we need to test the top limbs separately.
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
    assert(bitwidth() == rhs.bitwidth());
    return ucmpImpl(limbPtr(), numLimbs(), rhs.limbPtr(), rhs.numLimbs());
}

int APInt::ucmp(uint64_t rhs) const {
    if (numLimbs() == 1) {
        rhs &= topLimbMask();
    }
    return ucmpImpl(limbPtr(), numLimbs(), &rhs, 1);
}

static char intToSymbol(Limb l) {
    if (l < 10) {
        return static_cast<char>('0' + static_cast<int>(l));
    }
    return static_cast<char>('A' + static_cast<int>(l) - 10);
}

std::string APInt::toString(int b) const& {
    return APInt(*this).toString(b);
}

std::string APInt::toString(int b)&& {
    assert(b >= 2);
    assert(b <= 36);
    zext(std::max<size_t>(8, bitwidth()));
    std::string res;
    APInt const base(uint64_t(b), bitwidth());
    while (ucmp(0) != 0) {
        auto [q, r] = udivrem(*this, base);
        assert(r.ucmp(base) < 0);
        res.push_back(intToSymbol(*r.limbPtr()));
        *this = std::move(q);
    }
    std::reverse(res.begin(), res.end());
    if (res.empty()) {
        return "0";
    }
    return res;
}

std::string APInt::signedToString(int b) const {
    bool const neg = negative();
    if (!neg) {
        return toString();
    }
    auto negative = APMath::negate(*this);
    negative.sext(std::max<size_t>(8, negative.bitwidth()));
    auto res = negative.toString();
    res.insert(res.begin(), '-');
    return res;
}

static char charDigitToInt(char d) {
    if (d >= '0' && d <= '9') {
        return d - '0';
    }
    char const D = static_cast<char>(std::toupper(d));
    return D - 'A' + 10;
}

static void div2(std::vector<char>& str, int base) {
    int const halfBase = base / 2;
    int nextAdditive = 0;
    for (char& d: str) {
        int const additive = nextAdditive;
        nextAdditive = d % 2 ? halfBase : 0;
        d = static_cast<char>(d / 2 + additive);
    }
    if (str.size() > 1 && str[0] == 0) {
        str.erase(str.begin());
    }
}

static int isDigit(char c, int base) {
    assert(base >= 2);
    assert(base <= 36);
    if (c >= '0' && c <= std::min(+'9', '0' + base - 1)) {
        return true;
    }
    if (base <= 10) {
        return false;
    }
    if (c >= 'a' && c <= std::min(+'z', 'a' + base - 1)) {
        return true;
    }
    if (c >= 'A' && c <= std::min(+'Z', 'A' + base - 1)) {
        return true;
    }
    return false;
}

static int extractSign(std::string_view s, int base) {
    for (char c: s) {
        if (isDigit(c, base)) {
            return 1;
        }
        if (c == '-') {
            return -1;
        }
    }
    return 0;
}

std::optional<APInt> APInt::parse(std::string_view s, int base) {
    assert(base >= 2);
    assert(base <= 36);
    int sign = extractSign(s, base);
    if (sign == 0) {
        return std::nullopt;
    }
    std::vector<char> str;
    str.reserve(s.size());
    for (auto c: s) {
        if (isDigit(c, base)) {
            str.push_back(charDigitToInt(c));
        }
    }
    size_t bitwidth = 0;
    APInt res(limbBitSize);
    Limb* l = res.limbPtr();
    while (str.size() > 1 || str[0] != 0) {
        ++bitwidth;
        if (bitwidth > res.bitwidth()) {
            res.zext(2 * res.bitwidth());
            l = res.limbPtr();
        }
        l[(bitwidth - 1) / limbBitSize] |=
            Limb(str.back() % 2) << ((bitwidth - 1) % limbBitSize);
        div2(str, base);
    }
    if (res.ucmp(0) == 0) {
        sign = 1;
    }
    res.zext(std::max(size_t(1), bitwidth + (sign == -1)));
    if (sign == -1) {
        res.negate();
    }
    return res;
}

constexpr size_t initSeed = 0x9e3779b97f4a7c15;

void hashCombine(std::size_t& seed, Limb v) {
    seed ^= v + initSeed + (seed << 6) + (seed >> 2);
}

size_t APInt::hash() const {
    size_t seed = initSeed;
    for (Limb const l: limbs()) {
        hashCombine(seed, l);
    }
    return seed;
}

APInt::Limb* APInt::allocate(size_t numLimbs) {
    return static_cast<Limb*>(std::malloc(numLimbs * limbSize));
}

void APInt::deallocate(Limb* ptr, size_t numLimbs) {
    std::free(ptr);
}
