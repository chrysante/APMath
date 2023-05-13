#include <APMath/APFloat.h>

#include <cassert>
#include <cmath>
#include <functional>
#include <sstream>

using namespace APMath;

APFloat APMath::add(APFloat lhs, APFloat const& rhs) { return lhs.add(rhs); }

APFloat APMath::sub(APFloat lhs, APFloat const& rhs) { return lhs.sub(rhs); }

APFloat APMath::mul(APFloat const& lhs, APFloat const& rhs) {
    auto l = lhs;
    return l.mul(rhs);
}

APFloat APMath::div(APFloat const& lhs, APFloat const& rhs) {
    auto l = lhs;
    return l.div(rhs);
}

APFloat APMath::negate(APFloat operand) { return operand.negate(); }

APFloat APMath::precisionCast(APFloat operand, APFloatPrec precision) {
    return operand.setPrecision(precision);
}

int APMath::cmp(APFloat const& lhs, APFloat const& rhs) { return lhs.cmp(rhs); }

int APMath::cmp(APFloat const& lhs, double rhs) { return lhs.cmp(rhs); }

static bool isSinglePrec(auto const& arg, auto const&...) {
    return arg.precision() == APFloatPrec::Single;
}

static APFloat elemMathImpl(auto impl, auto const&... args) {
    if (isSinglePrec(args...)) {
        return APFloat(impl(args.template to<float>()...), APFloatPrec::Single);
    }
    else {
        return APFloat(impl(args.template to<double>()...),
                       APFloatPrec::Double);
    }
}

#define ELEM_MATH_STD_IMPL(name) [](auto... args) { return std::name(args...); }

APFloat APMath::abs(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(abs), arg);
}

APFloat APMath::exp(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(exp), arg);
}

APFloat APMath::exp2(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(exp2), arg);
}

APFloat APMath::exp10(APFloat const& arg) {
    return elemMathImpl([]<typename T>(T arg) { return std::pow(arg, T(10)); },
                        arg);
}

APFloat APMath::log(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(log), arg);
}

APFloat APMath::log2(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(log2), arg);
}

APFloat APMath::log10(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(log10), arg);
}

APFloat APMath::pow(APFloat const& base, APFloat const& exp) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(pow), base, exp);
}

APFloat APMath::sqrt(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(sqrt), arg);
}

APFloat APMath::cbrt(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(cbrt), arg);
}

APFloat APMath::hypot(APFloat const& a, APFloat const& b) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(hypot), a, b);
}

APFloat APMath::sin(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(sin), arg);
}

APFloat APMath::cos(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(cos), arg);
}

APFloat APMath::tan(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(tan), arg);
}

APFloat APMath::asin(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(asin), arg);
}

APFloat APMath::acos(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(acos), arg);
}

APFloat APMath::atan(APFloat const& arg) {
    return elemMathImpl(ELEM_MATH_STD_IMPL(atan), arg);
}

APFloatPrec const APFloatPrec::Single = { .mantissaWidth = 23,
                                          .exponentWidth = 8 };

APFloatPrec const APFloatPrec::Double = { .mantissaWidth = 52,
                                          .exponentWidth = 11 };

APFloat::APFloat(APFloatPrec precision): APFloat(0.0, precision) {}

APFloat::APFloat(long double value, APFloatPrec precision):
    _mantWidth(static_cast<uint32_t>(precision.mantissaWidth)),
    _expWidth(static_cast<uint32_t>(precision.exponentWidth)) {
    assert(precision == APFloatPrec::Single ||
           precision == APFloatPrec::Double);
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
    *this    = rhs;
    rhs      = tmp;
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
    _expWidth  = static_cast<uint32_t>(precision.exponentWidth);
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

int APFloat::cmp(double rhs) const { return cmp(APFloat(rhs, precision())); }

int APFloat::signbit() const {
    if (isSingle()) {
        return std::signbit(_f32);
    }
    else {
        return std::signbit(_f64);
    }
}

bool APFloat::isInf() const {
    if (isSingle()) {
        return std::isinf(_f32);
    }
    else {
        return std::isinf(_f64);
    }
}

bool APFloat::isNaN() const {
    if (isSingle()) {
        return std::isnan(_f32);
    }
    else {
        return std::isnan(_f64);
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

std::size_t APFloat::hash() const {
    if (isSingle()) {
        return std::hash<float>{}(_f32);
    }
    else {
        return std::hash<double>{}(_f64);
    }
}

std::optional<APFloat> APFloat::parse(std::string_view str,
                                      APFloatPrec precision) {
    char* ptr  = nullptr;
    double res = std::strtod(str.data(), &ptr);
    if (ptr == str.data()) {
        return std::nullopt;
    }
    return APFloat(res, precision);
}
