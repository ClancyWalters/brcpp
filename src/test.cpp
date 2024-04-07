#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "brc/brc.hpp"

#include <filesystem>
#include <print>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <exception>
#include <format>
#include <array>
#include <bit>
#include <string>
#include <string_view>

#include <absl/container/flat_hash_set.h>

const std::string test_directory = "../../../../res/samples/";

std::string retrieve(std::string name) {
    std::ifstream out_file(name);
    auto data = std::string((std::istreambuf_iterator<char>(out_file)), std::istreambuf_iterator<char>());
    out_file.close();
    return data;
}

TEST_CASE("measurements") {
    std::string name;
    SUBCASE("1") { name = "measurements-1";}
    SUBCASE("2") { name = "measurements-2";}
    SUBCASE("3") { name = "measurements-3";}
    SUBCASE("10") { name = "measurements-10";}
    SUBCASE("20") { name = "measurements-20";}
    SUBCASE("10000-unique-keys") { name = "measurements-10000-unique-keys";}
    SUBCASE("boundaries") { name = "measurements-boundaries";}
    SUBCASE("complex-utf8") { name = "measurements-complex-utf8";}
    SUBCASE("dot") { name = "measurements-dot";}
    SUBCASE("rounding") { name = "measurements-rounding";}
    SUBCASE("short") { name = "measurements-short";}
    SUBCASE("shortest") { name = "measurements-shortest";}

    CAPTURE(name);

    REQUIRE(std::freopen((test_directory + name + ".res").c_str(), "w", stdout));

    brc::execute(std::filesystem::path(test_directory + name + ".txt"));

    std::fclose(stdout);
    REQUIRE(std::freopen("CON", "w", stdout));

    auto res = retrieve(test_directory + name + ".res");
    auto gold = retrieve(test_directory + name + ".out");

    CHECK(res == gold);
}

TEST_CASE("findDelimiter") {
    union {
        unsigned char c[8];
        int64_t l;
    } data;
    data.l = 0UL;

    CHECK(std::format("{:064B}", data.l) == std::format("{:064B}", 0));

    data.c[3] = 0x0A;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", 0));

    data.l = 0UL;
    data.c[0] = 0x0A;
    data.c[1] = 0x0A;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", 0));

    data.l = 0UL;
    data.c[0] = 0x0A;
    data.c[1] = 0x0A;
    data.c[2] = 0x0A;
    data.c[3] = 0x0A;
    data.c[4] = 0x0A;
    data.c[5] = 0x0A;
    data.c[6] = 0x0A;
    data.c[7] = 0x0A;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", 0));

    data.l = 0UL;
    data.c[0] = 'a';
    data.c[1] = 'b';
    data.c[2] = 'c';
    data.c[3] = 'd';
    data.c[4] = 'e';
    data.c[5] = 'f';
    data.c[6] = 'g';
    data.c[7] = 'h';
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", 0));

    data.l = 0UL;
    data.c[0] = '\n';
    data.c[1] = '\n';
    data.c[2] = '\n';
    data.c[3] = '\n';
    data.c[4] = '\n';
    data.c[5] = '\n';
    data.c[6] = '\n';
    data.c[7] = '\n';
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", 0));

    uint64_t res = 1;
    res <<= 7 + 8 + 8;
    data.l = 0UL;
    data.c[0] = 'a';
    data.c[1] = 'b';
    data.c[2] = ';';
    data.c[3] = '-';
    data.c[4] = '1';
    data.c[5] = '2';
    data.c[6] = '.';
    data.c[7] = '3';
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res = 1;
    res <<= 7;
    data.l = 0UL;
    data.c[0] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res <<= 8;
    data.l = 0UL;
    data.c[1] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res <<= 8;
    data.l = 0UL;
    data.c[2] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res <<= 8;
    data.l = 0UL;
    data.c[3] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res <<= 8;
    data.l = 0UL;
    data.c[4] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res <<= 8;
    data.l = 0UL;
    data.c[5] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res <<= 8;
    data.l = 0UL;
    data.c[6] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));

    res <<= 8;
    data.l = 0UL;
    data.c[7] = 0x3B;
    CHECK(std::format("{:064B}", brc::internal::findDelimiter(data.l)) == std::format("{:064B}", res));
}

TEST_CASE("hash") {
    union {
        unsigned char c[16];
        int64_t l[2];
    } data;

    data.l[0] = 0; data.l[1] = 0;

    std::array<int, 2> index_counts = {0, 0};

    constexpr std::array<uint64_t, 9> mask1 = {
        0xFFUL,
        0xFFFFUL, 
        0xFFFFFFUL, 
        0xFFFFFFFFUL, 
        0xFFFFFFFFFFUL, 
        0xFFFFFFFFFFFFUL, 
        0xFFFFFFFFFFFFFFUL, 
        0xFFFFFFFFFFFFFFFFUL,
        0xFFFFFFFFFFFFFFFFUL
    };
    constexpr std::array<uint64_t, 9> mask2 = {0, 0, 0, 0, 0, 0, 0, 0, 0xFFFFFFFFFFFFFFFFL};

    //word = word & MASK1[count1]
    //word2 = MASK[2] & word2 & MASK1[count2]

    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 8);
    CHECK(index_counts[1] == 8);

    data.c[4] = ';';
    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 8 - 4);
    CHECK(index_counts[1] == 8);

    int pos = 0;
    data.l[0] = 0; data.l[1] = 0;
    data.c[pos] = ';';
    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 0);
    CHECK(index_counts[1] == 8);

    pos = 1;
    data.l[0] = 0; data.l[1] = 0;
    data.c[pos] = ';';
    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 1);
    CHECK(index_counts[1] == 8);

    pos = 7;
    data.l[0] = 0; data.l[1] = 0;
    data.c[pos] = ';';
    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 7);
    CHECK(index_counts[1] == 8);

    pos = 8;
    data.l[0] = 0; data.l[1] = 0;
    data.c[pos] = ';';
    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 8);
    CHECK(index_counts[1] == 0);

    pos = 9;
    data.l[0] = 0; data.l[1] = 0;
    data.c[pos] = ';';
    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 8);
    CHECK(index_counts[1] == 1);

    pos = 15;
    data.l[0] = 0; data.l[1] = 0;
    data.c[pos] = ';';
    index_counts[0] = std::countr_zero(brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero(brc::internal::findDelimiter(data.l[1])) >> 3;
    CHECK(index_counts[0] == 8);
    CHECK(index_counts[1] == 7);


    data.l[0] = 0; data.l[1] = 0;
    data.c[0] = 'a';
    data.c[1] = 'b';
    data.c[2] = ';';
    data.c[3] = '-';
    data.c[4] = '1';
    data.c[5] = '2';
    data.c[6] = '.';
    data.c[7] = '3';
    index_counts[0] = std::countr_zero((uint64_t)brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero((uint64_t)brc::internal::findDelimiter(data.l[1])) >> 3;

    int64_t mask = mask2[index_counts[0]];
    int64_t word = data.l[0] & mask1[index_counts[0]];
    int64_t word2 = mask & data.l[1] & mask1[index_counts[1]];

    CHECK(std::string((char*)&word, 3) == std::string("ab;"));
    CHECK(word2 == 0);

    data.l[0] = 0; data.l[1] = 0;
    data.c[0] = 'a';
    data.c[1] = 'b';
    data.c[2] = 'c';
    data.c[3] = 'd';
    data.c[4] = 'e';
    data.c[5] = 'f';
    data.c[6] = 'g';
    data.c[7] = 'h';
    data.c[8] = 'i';
    data.c[9] = ';';
    data.c[10] = '-';
    data.c[11] = '1';
    data.c[12] = '.';
    data.c[13] = '1';
    data.c[14] = '\n';
    data.c[15] = 'a';
    index_counts[0] = std::countr_zero((uint64_t)brc::internal::findDelimiter(data.l[0])) >> 3;
    index_counts[1] = std::countr_zero((uint64_t)brc::internal::findDelimiter(data.l[1])) >> 3;

    mask = mask2[index_counts[0]];
    word = data.l[0] & mask1[index_counts[0]];
    word2 = mask & data.l[1] & mask1[index_counts[1]];

    CHECK(std::string((char*)&word, 8) == std::string("abcdefgh"));
    CHECK(std::string((char*)&word2, 2) == std::string("i;"));

    //std::println("{} {:064B} & {:064B} = {:064B}", index_counts[0], (uint64_t)data.l[0], (uint64_t)mask1[index_counts[0]], (uint64_t)word);
    //std::println("{} {:064B} & {:064B} & {:064B} = {:064B}", index_counts[1], (uint64_t)mask, (uint64_t)data.l[1], (uint64_t)mask1[index_counts[1]], (uint64_t)word2);
}