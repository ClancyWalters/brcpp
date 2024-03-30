#include "brc/brc.hpp"

#include <chrono>
#include <print>
#include <filesystem>

int main(int argc, char *argv[]) {    
    auto start = std::chrono::high_resolution_clock::now();

    brc::execute(std::filesystem::path("../../../../res/measurements.txt"));

    auto stop = std::chrono::high_resolution_clock::now();

    std::println(stderr, "Total time: {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
}