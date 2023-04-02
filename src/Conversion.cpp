#include <APMath/Conversion.h>

#include <cassert>
#include <bit>

#include <APMath/APInt.h>
#include <APMath/APFloat.h>

using namespace APMath;

template <>
APInt APMath::bitcast(APFloat const& from) {
    if (from.precision() == APFloatPrec::Single) {
        return APInt(std::bit_cast<uint32_t>(from.to<float>()), 32);
    }
    else {
        return APInt(std::bit_cast<uint64_t>(from.to<double>()), 64);
    }
}

template <>
APFloat APMath::bitcast(APInt const& from) {
    assert((from.bitwidth() == 32 || from.bitwidth() == 64) &&
           "Other sizes are not supported by APFloat");
    if (from.bitwidth() == 32) {
        return APFloat(std::bit_cast<float>(from.to<uint32_t>()), APFloatPrec::Single);
    }
    else {
        return APFloat(std::bit_cast<double>(from.to<uint64_t>()), APFloatPrec::Double);
    }
}
