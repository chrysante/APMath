#include "Catch2.hpp"

#include <limits>
#include <vector>

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

TEST_CASE("ucmp - 3") {
    std::size_t const bitwidth0 = GENERATE(3, 7, 8, 32, 64, 127, 128, 256);
    std::size_t const bitwidth1 = GENERATE(64, 128);
    APInt const a(5, bitwidth0);
    APInt const b(std::numeric_limits<std::uint64_t>::max(), bitwidth0);
    APInt const c(5, bitwidth1);
    APInt const d(std::vector(2, std::numeric_limits<std::uint64_t>::max()), bitwidth1);
    INFO("Bitwidth0: " << bitwidth0);
    INFO("Bitwidth1: " << bitwidth1);
    CHECK(a.ucmp(b) < 0);
    CHECK(a.ucmp(c) == 0);
    CHECK(a.ucmp(d) < 0);
    CHECK(b.ucmp(a) > 0);
    CHECK(b.ucmp(c) > 0);
    if (bitwidth0 >= 64 && bitwidth1 == 64) {
        CHECK(b.ucmp(d) == 0);
    }
    else {
        CHECK(b.ucmp(d) < 0);
    }
    CHECK(c.ucmp(d) < 0);
}

TEST_CASE("scmp - 1") {
    auto const aVal = GENERATE(-100, -1, 0, 10, 100);
    auto const bVal = GENERATE(-100, -1, 0, 10, 100);
    auto const bitwidth = GENERATE(8, 63, 64, 65);
    APInt const a({ uint64_t(aVal), aVal >= 0 ? 0ull : -1ull }, bitwidth);
    APInt const b({ uint64_t(bVal), bVal >= 0 ? 0ull : -1ull }, bitwidth);
    if (aVal == bVal) {
        CHECK(a.scmp(b) == 0);
    }
    else if (aVal > bVal) {
        CHECK(a.scmp(b) > 0);
    }
    else {
        CHECK(a.scmp(b) < 0);
    }
}

TEST_CASE("ucmp - 4") {
    APInt const a(195, 64);
    APInt const b(99999, 64);
    CHECK(a.ucmp(b) < 0);
}

TEST_CASE("add - 1") {
    APInt a(5, 64);
    APInt b(6, 64);
    SECTION("a + b") {
        a.add(b);
        CHECK(a.ucmp(11) == 0);
    }
    SECTION("b + a") {
        b.add(a);
        CHECK(b.ucmp(11) == 0);
    }
}

TEST_CASE("add - overflow - 1") {
    APInt a({ 0xF000'0000'0000'0000, 1 }, 128);
    APInt b({ 0x1000'0000'0000'0000, 1 }, 128);
    APInt const ref({ 0, 3 }, 128);
    SECTION("a + b") {
        a.add(b);
        CHECK(a.ucmp(ref) == 0);
    }
    SECTION("b + a") {
        b.add(a);
        CHECK(b.ucmp(ref) == 0);
    }
}

TEST_CASE("add - overflow - 2") {
    APInt       a({ 0xFFFF'FFFF'FFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF, 0x7FFF'FFFF'FFFF'FFFF }, 192);
    APInt const b(1, 1);
    APInt const c({ 0, 0, 0x8000'0000'0000'0000 }, 192);
    a.add(b);
    CHECK(a.ucmp(c) == 0);
}

TEST_CASE("sub - 1") {
    APInt a(6, 64);
    APInt b(5, 64);
    SECTION("a - b") {
        a.sub(b);
        CHECK(a.ucmp(1) == 0);
    }
    SECTION("b - a") {
        b.sub(a);
        CHECK(b.ucmp(0xFFFF'FFFF'FFFF'FFFF) == 0);
    }
}

TEST_CASE("sub - 2") {
    APInt a({ 3, 2 }, 128);
    APInt b({ 5, 1 }, 128);
    APInt const ref({ APInt::Limb(-2), 0 }, 128);
    a.sub(b);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("sub - 3") {
    APInt a({ 5, 2 }, 128);
    APInt b({ 3, 1 }, 128);
    APInt const ref({ 2, 1 }, 128);
    a.sub(b);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("sub - underflow - 1") {
    auto const bWidth = GENERATE(1, 64, 192);
    APInt       a(0, 192);
    APInt const b(1, bWidth);
    APInt const c({ 0xFFFF'FFFF'FFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF }, 192);
    a.sub(b);
    CHECK(a.ucmp(c) == 0);
}

TEST_CASE("mul - 1") {
    APInt a(5, 64);
    APInt b(6, 64);
    SECTION("a * b") {
        a.mul(b);
        CHECK(a.ucmp(30) == 0);
    }
    SECTION("b * a") {
        b.mul(a);
        CHECK(b.ucmp(30) == 0);
    }
}

TEST_CASE("mul - 2") {
    APInt a(0x8000'0000'0000'0000, 128);
    APInt b(3, 128);
    APInt ref({ 0x8000'0000'0000'0000, 1 }, 128);
    SECTION("a * b") {
        a.mul(b);
        CHECK(a.ucmp(ref) == 0);
    }
    SECTION("b * a") {
        b.mul(a);
        CHECK(b.ucmp(ref) == 0);
    }
}

TEST_CASE("mul - 3") {
    APInt a(-1, 64);
    APInt b(-1, 64);
    APInt ref(uint64_t(-1) * uint64_t(-1), 64);
    a.mul(b);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("mul - 4") {
    APInt a(-4, 64);
    APInt b(7, 64);
    APInt ref(uint64_t(-28), 64);
    a.mul(b);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("udiv - 1") {
    size_t const aVal = GENERATE(0, 1, 7, 10, 100, 99999, 0xFFFF'FFFF'FFFF'FFFFull);
    size_t const bVal = GENERATE(1, 2, 7, 99999, 0xFFFF'FFFF'FFFF'FFFFull);
    auto const bitwidth = GENERATE(64, 65, 127, 128);
    APInt a(aVal, bitwidth);
    APInt b(bVal, bitwidth);
    auto const [q, r] = udivmod(a, b);
    INFO("a = " << aVal << "\nb = " << bVal << "\nbitwidth = " << bitwidth <<
         "\na / b = " << aVal / bVal << "\na % b = " << aVal % bVal);
    CHECK(q.ucmp(aVal / bVal) == 0);
    CHECK(r.ucmp(aVal % bVal) == 0);
}

TEST_CASE("lshl - 1") {
    APInt a(6, 64);
    a.lshl(1);
    CHECK(a.ucmp(12) == 0);
    a.lshl(1);
    CHECK(a.ucmp(24) == 0);
    a.lshl(2);
    CHECK(a.ucmp(96) == 0);
}

TEST_CASE("lshl - 2") {
    APInt a(0xACAB'DEAD'BEEF'ACAB, 128);
    a.lshl(32);
    APInt const ref({ 0xBEEF'ACAB'0000'0000, 0xACAB'DEAD }, 128);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("lshl - 3") {
    APInt a({ 1, 0 }, 128);
    a.lshl(64);
    APInt const ref({ 0, 1 }, 128);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("lshl - 4") {
    APInt a({ 0xDEAD'BEEF, 0, 0, 0 }, 200);
    a.lshl(132);
    APInt const ref({ 0, 0, 0xDEAD'BEEF'0, 0 }, 200);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("lshr - 1") {
    APInt a(72, 64);
    a.lshr(1);
    CHECK(a.ucmp(36) == 0);
    a.lshr(1);
    CHECK(a.ucmp(18) == 0);
    a.lshr(2);
    CHECK(a.ucmp(4) == 0);
}

TEST_CASE("lshr - 2") {
    APInt a(0xACAB'DEAD'BEEF'ACAB, 128);
    a.lshr(32);
    APInt const ref(0xACAB'DEAD, 128);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("lshr - 3") {
    APInt a({ 0, 1 }, 128);
    a.lshr(64);
    APInt const ref({ 1, 0 }, 128);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("lshr - 4") {
    APInt a({ 0, 0, 0xDEAD'BEEF, 0 }, 200);
    a.lshr(132);
    APInt const ref({ 0xDEAD'BEE, 0, 0, 0 }, 200);
    CHECK(a.ucmp(ref) == 0);
}
