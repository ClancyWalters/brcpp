#pragma once

#include <filesystem>
#include <limits>

namespace brc {
    struct Station {
        int32_t min = 1000;
        int32_t max = -1000;
        int32_t total = 0;
        uint32_t count = 0;
    };

    void execute(std::filesystem::path file_path);
}
