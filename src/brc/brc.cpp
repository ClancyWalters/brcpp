#include "brc.hpp"

#include <unordered_map>
#include <string>
#include <fstream>
#include <set>
#include <ranges>
#include <print>

#include <mio/mmap.hpp>

auto brc::execute(std::filesystem::path file_path) -> void {
    
    auto stations = std::unordered_map<std::string, brc::Station>{};

    auto mmap = mio::mmap_source(file_path.string());
    auto size = mmap.mapped_length();

    size_t ptr = 0;
    size_t start = 0;
    int32_t val = 0;
    while (ptr < size - 1) {
        start = ptr;
        while (mmap[ptr] != ';') { ptr++; }
        auto name = std::string(&mmap[start], ptr++ - start);
        
        bool negative = mmap[ptr] == '-';
        ptr += negative;
        if (mmap[ptr + 1] == '.') {
            val = (mmap[ptr] - '0') * 10 + mmap[ptr + 2] - '0';
            ptr += 4;
        } else {
            val = (mmap[ptr] - '0') * 100 + (mmap[ptr + 1] - '0') * 10 + mmap[ptr + 3] - '0';
            ptr += 5;
        }
        val *= negative * -2 + 1;

        auto& station = stations[name];

        station.count += 1;
        station.total += val;

        station.max = (val + station.max + std::abs(val - station.max)) / 2;
        station.min = (val + station.min - std::abs(val - station.min)) / 2;

        /*if (val > station.max) {
            station.max = val;
        } 
        if (val < station.min) {
            station.min = val;
        }*/
    }
    mmap.unmap();
    
    auto station_names = std::set<std::string>();
    station_names.insert_range(std::views::keys(stations));

    std::print("{}", "{");
    for (const auto& [idx, key] : std::views::enumerate(station_names)) {
        auto min = std::ceil(stations[key].min) / 10.0;
        auto average = std::ceil(((double) stations[key].total / stations[key].count)) / 10.0;
        auto max = std::ceil(stations[key].max) / 10.0;

        if (idx != 0) {
            std::print("{}", ", ");
        }
        
        std::print("{}={:.1f}/{:.1f}/{:.1f}", key, min, average, max);
    }
    std::println("{}", "}");
}