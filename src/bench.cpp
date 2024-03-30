#include "brc/brc.hpp"

#include <filesystem>

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

int main(int argc, char *argv[]) {    
    ankerl::nanobench::Bench().epochs(10).run("benchmark", [&] {
        brc::execute(std::filesystem::path("../../../../res/measurements.txt"));
    });
}