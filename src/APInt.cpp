#include "APInt.h"

#include <cassert>

using namespace APMath;

static std::size_t ceilDiv(std::size_t a, std::size_t b) {
    return a / b + !!(a % b);
}

APInt::APInt(std::size_t bitwidth): APInt(0, bitwidth) {}

APInt::APInt(std::uint64_t value, std::size_t bitwidth) {
    assert(bitwidth <= maxBitwidth());
    _bitwidth = static_cast<uint32_t>(bitwidth);
    _numLimbs = _bitwidth / limbSize;
    isLocal = _numLimbs == 1;
}

APInt::APInt(APInt const& rhs) {
    
}

APInt::APInt(APInt&& rhs) noexcept {
    
}

APInt& APInt::operator=(APInt const& rhs) {
    
    return *this;
}

APInt& APInt::operator=(APInt&& rhs) noexcept {
    
    return *this;
}

APInt::~APInt() {
    
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

int APInt::ucmp(APInt const& rhs) const {
    
    return 0;
}

int APInt::ucmp(std::uint64_t rhs) const {
    
    return 0;
}
