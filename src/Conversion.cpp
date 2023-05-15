#include <APMath/Conversion.h>

#include <bit>
#include <cassert>

#include <APMath/APFloat.h>
#include <APMath/APInt.h>

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
        return APFloat(std::bit_cast<float>(from.to<uint32_t>()),
                       APFloatPrec::Single);
    }
    else {
        return APFloat(std::bit_cast<double>(from.to<uint64_t>()),
                       APFloatPrec::Double);
    }
}

template <>
APInt APMath::valuecast(APFloat const& from, size_t toBitwidth) {
    if (from.precision() == APFloatPrec::Single) {
        switch (toBitwidth) {
        case 8:
            return APInt(static_cast<uint8_t>(from.to<float>()), 8);
        case 16:
            return APInt(static_cast<uint16_t>(from.to<float>()), 16);
        case 32:
            return APInt(static_cast<uint32_t>(from.to<float>()), 32);
        case 64:
            return APInt(static_cast<uint64_t>(from.to<float>()), 64);
        default:
            assert(false);
            return APInt{};
        }
    }
    else {
        switch (toBitwidth) {
        case 8:
            return APInt(static_cast<uint8_t>(from.to<double>()), 8);
        case 16:
            return APInt(static_cast<uint16_t>(from.to<double>()), 16);
        case 32:
            return APInt(static_cast<uint32_t>(from.to<double>()), 32);
        case 64:
            return APInt(static_cast<uint64_t>(from.to<double>()), 64);
        default:
            assert(false);
            return APInt{};
        }
    }
}

template <>
APFloat APMath::valuecast(APInt const& from, size_t toBitwidth) {
    assert((toBitwidth == 32 || toBitwidth == 64) &&
           "Other sizes are not supported by APFloat");
    APInt ext = zext(from, 64);
    if (toBitwidth == 32) {
        return APFloat(static_cast<float>(ext.to<uint64_t>()),
                       APFloatPrec::Single);
    }
    else {
        return APFloat(static_cast<double>(ext.to<uint64_t>()),
                       APFloatPrec::Double);
    }
}

template <>
APInt APMath::signedValuecast(APFloat const& from, size_t toBitwidth) {
    if (from.precision() == APFloatPrec::Single) {
        switch (toBitwidth) {
        case 8:
            return APInt(static_cast<int8_t>(from.to<float>()), 8);
        case 16:
            return APInt(static_cast<int16_t>(from.to<float>()), 16);
        case 32:
            return APInt(static_cast<int32_t>(from.to<float>()), 32);
        case 64:
            return APInt(static_cast<int64_t>(from.to<float>()), 64);
        default:
            assert(false);
            return APInt{};
        }
    }
    else {
        switch (toBitwidth) {
        case 8:
            return APInt(static_cast<int8_t>(from.to<double>()), 8);
        case 16:
            return APInt(static_cast<int16_t>(from.to<double>()), 16);
        case 32:
            return APInt(static_cast<int32_t>(from.to<double>()), 32);
        case 64:
            return APInt(static_cast<int64_t>(from.to<double>()), 64);
        default:
            assert(false);
            return APInt{};
        }
    }
}

template <>
APFloat APMath::signedValuecast(APInt const& from, size_t toBitwidth) {
    assert((toBitwidth == 32 || toBitwidth == 64) &&
           "Other sizes are not supported by APFloat");
    APInt ext = sext(from, 64);
    if (toBitwidth == 32) {
        return APFloat(static_cast<float>(ext.to<int64_t>()),
                       APFloatPrec::Single);
    }
    else {
        return APFloat(static_cast<double>(ext.to<int64_t>()),
                       APFloatPrec::Double);
    }
}
