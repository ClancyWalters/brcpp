#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "brc/brc.hpp"

#include <filesystem>
#include <print>
#include <sstream>
#include <streambuf>
#include <iostream>
#include <exception>

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