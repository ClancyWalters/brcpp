#pragma once

#include <filesystem>
#include <limits>

namespace brc {
    struct Station {
        int16_t min = 999;
        int16_t max = -999;
        uint32_t count = 0;
        int64_t total = 0;
    };

    void execute(std::filesystem::path file_path);
}
