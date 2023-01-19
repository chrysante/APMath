#include "Catch2.hpp"

#include <APMath/APInt.h>

using namespace APMath;

TEST_CASE("ucmp - 1") {
    std::size_t const bitwidth = GENERATE(1, 7, 8, 32, 64, 127, 128, 256);
    APInt const i(bitwidth);
    CHECK(i.ucmp(0) == 0);
}

TEST_CASE("ucmp - 2") {
    std::size_t const bitwidth = GENERATE(3, 7, 8, 32, 64, 127, 128, 256);
    APInt const i(5, bitwidth);
    INFO("Bitwidth: " << bitwidth);
    CHECK(i.ucmp(5) == 0);
    CHECK(i.ucmp(0) > 0);
    CHECK(i.ucmp(10) < 0);
}

TEST_CASE("Addition") {
    
}

