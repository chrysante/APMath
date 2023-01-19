#include "Catch2.hpp"

#include <APMath/APInt.h>

using namespace APMath;

TEST_CASE("Construction") {
    APInt i(32);
    CHECK(i.scmp(0) == 0);
    CHECK(i.ucmp(0) == 0);
}

TEST_CASE("Addition") {
    CHECK(false);
}
