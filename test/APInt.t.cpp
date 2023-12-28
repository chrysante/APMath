#include "Catch2.hpp"

#include <limits>
#include <vector>

#include <APMath/APInt.h>

using namespace APMath;

TEST_CASE("Lifetime") {
    size_t const bitwidth =
        GENERATE(1u, 7u, 8u, 32u, 64u, 65u, 127u, 128u, 256u);
    uint64_t const lowWord =
        GENERATE(static_cast<uint64_t>(-100), 0xDEAD'BEEF, 0u, 1u, 997u);
    uint64_t const highWord = GENERATE(-100u, 0xDEAD'BEEF, 0u, 1u, 997u);
    APInt const a({ lowWord, highWord }, bitwidth);
    SECTION("Copy construction") {
        APInt const b = a;
        CHECK(a.ucmp(b) == 0);
    }
    SECTION("Move construction") {
        APInt tmp = a;
        APInt b   = std::move(tmp);
        CHECK(a.ucmp(b) == 0);
    }
    SECTION("Copy assignment") {
        auto const bWidth = GENERATE(1u, 128u);
        APInt b(bWidth);
        b = a;
        CHECK(a.ucmp(b) == 0);
    }
    SECTION("Move assignment") {
        auto const bWidth = GENERATE(1u, 128u);
        APInt b(bWidth);
        APInt tmp = a;
        b         = std::move(tmp);
        CHECK(a.ucmp(b) == 0);
    }
}

TEST_CASE("ucmp - 1") {
    std::size_t const bitwidth =
        GENERATE(1u, 7u, 8u, 32u, 64u, 127u, 128u, 256u);
    APInt const i(bitwidth);
    CHECK(i.ucmp(0) == 0);
}

TEST_CASE("ucmp - 2") {
    std::size_t const bitwidth =
        GENERATE(3u, 7u, 8u, 32u, 64u, 127u, 128u, 256u);
    APInt const i(5, bitwidth);
    INFO("Bitwidth: " << bitwidth);
    CHECK(i.ucmp(5) == 0);
    CHECK(i.ucmp(0) > 0);
    if (bitwidth >= 4) {
        CHECK(i.ucmp(10) < 0);
    }
}

TEST_CASE("ucmp - 3") {
    std::size_t const bitwidth =
        GENERATE(3u, 7u, 8u, 32u, 64u, 127u, 128u, 256u);
    APInt const a(5, bitwidth);
    APInt const b(std::numeric_limits<std::uint64_t>::max(), bitwidth);
    APInt const c(5, bitwidth);
    APInt const d(std::vector(2, std::numeric_limits<std::uint64_t>::max()),
                  bitwidth);

    CHECK(a.ucmp(b) < 0);
    CHECK(a.ucmp(c) == 0);
    CHECK(a.ucmp(d) < 0);
    CHECK(b.ucmp(a) > 0);
    CHECK(b.ucmp(c) > 0);
    if (bitwidth <= 64) {
        CHECK(b.ucmp(d) == 0);
    }
    else {
        CHECK(b.ucmp(d) < 0);
    }
    CHECK(c.ucmp(d) < 0);
}

TEST_CASE("scmp - 1") {
    int64_t const aVal    = GENERATE(-100, -1, 0, 10, 100);
    int64_t const bVal    = GENERATE(-100, -1, 0, 10, 100);
    size_t const bitwidth = GENERATE(8u, 63u, 64u, 65u);
    APInt const a({ uint64_t(aVal), aVal >= 0 ? 0ull : -1ull }, bitwidth);
    APInt const b({ uint64_t(bVal), bVal >= 0 ? 0ull : -1ull }, bitwidth);
    if (aVal == bVal) {
        CHECK(a.scmp(b) == 0);
    }
    else if (aVal > bVal) {
        CHECK(a.scmp(b) > 0);
    }
    else {
        INFO("a = " << a.signedToString() << "\nb = " << b.signedToString());
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
    APInt a({ 0xFFFF'FFFF'FFFF'FFFF,
              0xFFFF'FFFF'FFFF'FFFF,
              0x7FFF'FFFF'FFFF'FFFF },
            192);
    APInt const b(1, 192);
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
    APInt a(0, 192);
    APInt const b(1, 192);
    APInt const c({ 0xFFFF'FFFF'FFFF'FFFF,
                    0xFFFF'FFFF'FFFF'FFFF,
                    0xFFFF'FFFF'FFFF'FFFF },
                  192);
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
    APInt a(uint64_t(-1), 64);
    APInt b(uint64_t(-1), 64);
    APInt ref(uint64_t(-1) * uint64_t(-1), 64);
    a.mul(b);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("mul - 4") {
    APInt a(uint64_t(-4), 64);
    APInt b(7, 64);
    APInt ref(uint64_t(-28), 64);
    a.mul(b);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("udivrem - 1") {
    uint64_t const aVal =
        GENERATE(0u, 1u, 7u, 10u, 100u, 99999u, 0xFFFF'FFFF'FFFF'FFFFull);
    uint64_t const bVal =
        GENERATE(1u, 2u, 7u, 99999u, 0xFFFF'FFFF'FFFF'FFFFull);
    auto const bitwidth = GENERATE(64u, 65u, 127u, 128u);
    APInt const a(aVal, bitwidth);
    APInt const b(bVal, bitwidth);
    auto const [q, r] = udivrem(a, b);
    CHECK(q.ucmp(aVal / bVal) == 0);
    CHECK(r.ucmp(aVal % bVal) == 0);
}

TEST_CASE("sdivrem - 1") {
    int64_t const aVal    = GENERATE(-100, 0, 1, 7, 10, 100, 99999);
    int64_t const bVal    = GENERATE(-100, 1, 2, 7, 99999);
    size_t const bitwidth = GENERATE(64u, 65u, 127u, 128u);
    APInt a(uint64_t(aVal), 64);
    a.sext(bitwidth);
    APInt b(uint64_t(bVal), 64);
    b.sext(bitwidth);
    auto const [q, r]  = sdivrem(a, b);
    int64_t const qVal = aVal / bVal;
    APInt qRef(uint64_t(qVal), 64);
    qRef.sext(bitwidth);
    int64_t const rVal = aVal % bVal;
    APInt rRef(uint64_t(rVal), 64);
    rRef.sext(bitwidth);
    CHECK(q.ucmp(qRef) == 0);
    CHECK(r.ucmp(rRef) == 0);
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

TEST_CASE("ashr - 1") {
    size_t const bitwidth = GENERATE(14u, 32u, 64u, 65u, 128u);
    APInt a(uint64_t(-64), 64);
    a.sext(bitwidth);
    a.ashr(2);
    APInt ref(uint64_t(-16), 64);
    ref.sext(bitwidth);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("negate - 1") {
    uint64_t const aVal   = GENERATE(-100u, uint64_t(-1), 0u, 1u, 100u);
    size_t const bitwidth = GENERATE(64u, 65u, 127, 128u);
    APInt a(aVal, 64);
    a.sext(bitwidth);
    a.negate();
    APInt ref(-aVal, 64);
    ref.sext(bitwidth);
    CHECK(a.ucmp(ref) == 0);
}

TEST_CASE("Bitset operations") {
    APInt a(100);
    CHECK(a.popcount() == 0);
    CHECK(a.clz() == 100);
    CHECK(a.ctz() == 100);
    a.set(0);
    CHECK(a.limbs()[0] == 1);
    CHECK(a.popcount() == 1);
    CHECK(a.clz() == 99);
    CHECK(a.ctz() == 0);

    a.set(64);
    CHECK(a.limbs()[1] == 1);
    CHECK(a.popcount() == 2);
    CHECK(a.clz() == 35);
    CHECK(a.ctz() == 0);

    a.set(75);
    CHECK(a.limbs()[1] == 2049);
    CHECK(a.popcount() == 3);
    CHECK(a.clz() == 24);
    CHECK(a.ctz() == 0);

    a.set(0, false);
    CHECK(a.limbs()[0] == 0);
    CHECK(a.popcount() == 2);
    CHECK(a.clz() == 24);
    CHECK(a.ctz() == 64);

    a.set(10);
    CHECK(a.ctz() == 10);
    a.clear(10);

    a.clear(64);
    CHECK(a.limbs()[1] == 2048);
    CHECK(a.popcount() == 1);
    CHECK(a.clz() == 24);
    CHECK(a.ctz() == 75);

    a.clear(75);
    CHECK(a.limbs()[1] == 0);
    CHECK(a.popcount() == 0);
    CHECK(!a.any());
    CHECK(a.none());
    CHECK(a == 0);

    a.flip(75);
    CHECK(a.limbs()[1] == 2048);
    CHECK(a.any());
    CHECK(!a.none());

    a.flip();
    a.set(75);
    CHECK(a.all());
}

TEST_CASE("zext - 1") {
    APInt a(6, 3);
    a.zext(64);
    CHECK(a.ucmp(6) == 0);
    a.zext(128);
    CHECK(a.ucmp(6) == 0);
    a.zext(1);
    CHECK(a.ucmp(0) == 0);
}

TEST_CASE("sext - 1") {
    APInt a(6, 4);
    a.sext(64);
    CHECK(a.ucmp(6) == 0);
    a.sext(128);
    CHECK(a.ucmp(6) == 0);
    a.sext(1);
    CHECK(a.ucmp(0) == 0);
}

TEST_CASE("sext - 2") {
    APInt a(uint64_t(-6), 4);
    a.sext(64);
    CHECK(a.ucmp(uint64_t(-6)) == 0);
    a.sext(128);
    CHECK(a.ucmp(APInt({ static_cast<uint64_t>(-6), static_cast<uint64_t>(-1) },
                       128)) == 0);
    a.sext(1);
    CHECK(a.ucmp(0) == 0);
}

TEST_CASE("String conversion") {
    CHECK(APInt(5, 64).toString(16) == "5");
    CHECK(APInt(uint64_t(-1), 64).toString(16) == "FFFFFFFFFFFFFFFF");
    CHECK(APInt(uint64_t(-1), 64).sext(128).toString(16) ==
          "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    CHECK(APInt(128761486, 64).toString(10) == "128761486");
    CHECK(APInt(uint64_t(-5), 64).signedToString(10) == "-5");
    CHECK(APInt(0, 1).toString(10) == "0");
    CHECK(APInt(0, 1).signedToString(10) == "0");
    CHECK(APInt(0xFF, 11).toString(10) == "255");
    CHECK(APInt(0xFF, 64).toString(10) == "255");
    CHECK(APInt(0xFF, 64).signedToString(10) == "255");
}

TEST_CASE("String parse - 1") {
    APInt const a = APInt::parse(" - f'F", 16).value();
    CHECK(a.bitwidth() == 9);
    CHECK(a.ucmp(uint64_t(-255)) == 0);
    APInt const b = APInt::parse("0", 10).value();
    CHECK(b.bitwidth() == 1);
    CHECK(b.ucmp(0) == 0);
    APInt const c = APInt::parse("-0", 10).value();
    CHECK(c.bitwidth() == 1);
    CHECK(c.ucmp(0) == 0);
    CHECK(!APInt::parse("8", 8));
    CHECK(APInt::parse("H", 18).value().ucmp(17) == 0);
}

TEST_CASE("String parse - 2") {
    auto a = APInt::parse("-128", 10, 8);
    CHECK(!a);
    auto b = APInt::parse("-127", 10, 8).value();
    CHECK(b == 129); // 129 == -127 in 8 bit two's complement
}

TEST_CASE("Conversion to native") {
    APInt a(-123, 32);
    CHECK(a.to<int32_t>() == -123);
    APInt b(1024 + 255, 8);
    CHECK(b.to<int32_t>() == 255);
    APInt c = APInt::parse("1 0000 0000 0000 0123", 16, 128).value();
    CHECK(c.to<int64_t>() == 0x123);
#if defined(__SIZEOF_INT128__) && !defined(__linux__) 
    CHECK(c.to<__uint128_t>() - 0x124 == ~uint64_t(0));
#endif
    APInt d(0x1'0000'0123, 64);
    CHECK(d.to<int32_t>() == 0x123);
    APInt e(0x1'0000'0123, 32);
    CHECK(e.to<int64_t>() == 0x123);
}
