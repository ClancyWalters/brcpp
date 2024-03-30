#include "brc.hpp"

#include <unordered_map>
#include <string>
#include <fstream>
#include <set>
#include <ranges>
#include <print>

#include <mio/mmap.hpp>
#include <absl/container/node_hash_map.h>

constexpr auto THREAD_COUNT = 6;

absl::node_hash_map<std::string, brc::Station> processBlock(mio::mmap_source& mmap, size_t start, size_t len) {
    auto stations = absl::node_hash_map<std::string, brc::Station>{};

    size_t endpoint = start + len;
    size_t ptr = start;

    if (endpoint < mmap.mapped_length() && mmap[endpoint] != '\n') {
        while (mmap[endpoint] != '\n') {
            endpoint++;
        }
    }

    if (ptr != 0) {
        while (mmap[ptr++] != '\n') {}
    }

    int64_t val;
    while (ptr < endpoint - 1) {
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
        
        val *= negative ? -1 : 1;

        auto& station = stations[name];

        station.count += 1;
        station.total += val;

        if (val > station.max) {
            station.max = val;
        } 
        if (val < station.min) {
            station.min = val;
        }
    }

    return std::move(stations);
}

auto brc::execute(std::filesystem::path file_path) -> void {
    
    auto stations = absl::node_hash_map<std::string, brc::Station>{};

    auto mmap = mio::mmap_source(file_path.string());
    auto size = mmap.mapped_length();

    auto block_size = size / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
        auto station_section = processBlock(mmap, i * block_size, block_size);

        for (const auto& pair : station_section) {
            stations[pair.first].count += pair.second.count;
            stations[pair.first].total += pair.second.total;
            if (pair.second.max > stations[pair.first].max) {
                stations[pair.first].max = pair.second.max;
            } 
            if (pair.second.min < stations[pair.first].min) {
                stations[pair.first].min = pair.second.min;
            }
        }
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