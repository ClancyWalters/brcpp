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
    size_t line_start = 0;
    while (ptr < size - 1) {
        line_start = ptr;
        while (mmap[ptr] != ';') { ptr++; }
        auto line = std::string(&mmap[line_start], ptr++ - line_start);
        
        auto split_point = line.find(";");
        auto name = line.substr(0, split_point);
        auto val = std::stod(line.substr(split_point+1, line.length()));

        if (stations.contains(name)) {
            auto& station = stations[name];
            station.count += 1;
            station.total += val;

            if (val > station.max) {
                station.max = val;
            } 
            if (val < station.min) {
                station.min = val;
            }

        } else {
            stations[name] = Station { .min = val, .max = val, .total = val, .count = 1};
        }
    }
    mmap.unmap();
    
    auto station_names = std::set<std::string>();
    station_names.insert_range(std::views::keys(stations));

    std::print("{}", "{");
    for (const auto& [idx, key] : std::views::enumerate(station_names)) {
        auto min = std::ceil(stations[key].min * 10.0) / 10.0;
        auto average = std::ceil(((double) stations[key].total / stations[key].count) * 10) / 10.0;
        auto max = std::ceil(stations[key].max * 10.0) / 10.0;

        if (idx != 0) {
            std::print("{}", ", ");
        }
        
        std::print("{}={:.1f}/{:.1f}/{:.1f}", key, min, average, max);
    }
    std::println("{}", "}");
}