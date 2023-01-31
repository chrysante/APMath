#include <APMath/APFloat.h>

#include <cassert>
#include <sstream>

using namespace APMath;

APFloat APMath::add(APFloat lhs, APFloat const& rhs) {
    return lhs.add(rhs);
}

APFloat APMath::sub(APFloat lhs, APFloat const& rhs) {
    return lhs.sub(rhs);
}

APFloat APMath::mul(APFloat const& lhs, APFloat const& rhs) {
    auto l = lhs;
    return l.mul(rhs);
}

APFloat APMath::div(APFloat const& lhs, APFloat const& rhs) {
    auto l = lhs;
    return l.div(rhs);
}

APFloat APMath::negate(APFloat operand) {
    return operand.negate();
}

APFloat precisionCast(APFloat operand, APFloatPrec precision) {
    return operand.setPrecision(precision);
}

int cmp(APFloat const& lhs, APFloat const& rhs) {
    return lhs.cmp(rhs);
}

APFloatPrec const APFloatPrec::Single = { .mantissaWidth = 23, .exponentWidth = 8 };

APFloatPrec const APFloatPrec::Double = { .mantissaWidth = 52, .exponentWidth = 11 };

APFloat::APFloat(APFloatPrec precision): APFloat(0.0, precision) {}

APFloat::APFloat(long double value, APFloatPrec precision):
    _mantWidth(static_cast<uint32_t>(precision.mantissaWidth)),
    _expWidth(static_cast<uint32_t>(precision.exponentWidth))
{
    assert(precision == APFloatPrec::Single || precision == APFloatPrec::Double);
    if (isSingle()) {
        _f32 = static_cast<float>(value);
    }
    else {
        _f64 = static_cast<double>(value);
    }
}

APFloat::APFloat(APFloat const& rhs) = default;

APFloat::APFloat(APFloat&& rhs) noexcept = default;

APFloat& APFloat::operator=(APFloat const& rhs) = default;

APFloat& APFloat::operator=(APFloat&& rhs) noexcept = default;

APFloat::~APFloat() = default;

void APFloat::swap(APFloat& rhs) noexcept {
    auto tmp = *this;
    *this = rhs;
    rhs = tmp;
}

APFloat& APFloat::add(APFloat const& rhs) {
    assert(precision() == rhs.precision());
    if (precision() == APFloatPrec::Single) {
        _f32 += rhs._f32;
    }
    else {
        _f64 += rhs._f64;
    }
    return *this;
}

APFloat& APFloat::sub(APFloat const& rhs) {
    assert(precision() == rhs.precision());
    if (precision() == APFloatPrec::Single) {
        _f32 -= rhs._f32;
    }
    else {
        _f64 -= rhs._f64;
    }
    return *this;
}

APFloat& APFloat::mul(APFloat const& rhs) {
    assert(precision() == rhs.precision());
    if (precision() == APFloatPrec::Single) {
        _f32 *= rhs._f32;
    }
    else {
        _f64 *= rhs._f64;
    }
    return *this;
}

APFloat& APFloat::div(APFloat const& rhs) {
    assert(precision() == rhs.precision());
    if (precision() == APFloatPrec::Single) {
        _f32 /= rhs._f32;
    }
    else {
        _f64 /= rhs._f64;
    }
    return *this;
}

APFloat& APFloat::negate() {
    if (precision() == APFloatPrec::Single) {
        _f32 = -_f32;
    }
    else {
        _f64 = -_f64;
    }
    return *this;
}

APFloat& APFloat::setPrecision(APFloatPrec precision) {
    _mantWidth = static_cast<uint32_t>(precision.mantissaWidth);
    _expWidth = static_cast<uint32_t>(precision.exponentWidth);
    return *this;
}

int APFloat::cmp(APFloat const& rhs) const {
    assert(precision() == rhs.precision());
    if (isSingle()) {
        if (_f32 < rhs._f32) {
            return -1;
        }
        if (_f32 > rhs._f32) {
            return 1;
        }
        return 0;
    }
    else {
        if (_f64 < rhs._f64) {
            return -1;
        }
        if (_f64 > rhs._f64) {
            return 1;
        }
        return 0;
    }
}

std::string APFloat::toString() const {
    std::stringstream sstr;
    sstr << std::fixed;
    if (isSingle()) {
        sstr << _f32;
    }
    else {
        sstr << _f64;
    }
    return std::move(sstr).str();
}

std::optional<APFloat> APFloat::parse(std::string_view str, APFloatPrec precision) {
    char* ptr = nullptr;
    double res = std::strtod(str.data(), &ptr);
    if (ptr == str.data()) {
        return std::nullopt;
    }
    return APFloat(res, precision);
}
