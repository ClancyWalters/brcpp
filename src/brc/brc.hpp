#pragma once

#include <filesystem>
#include <limits>

namespace brc {
    struct Station {
        double min = 100;
        double max = -100;
        double total = 0;
        uint32_t count = 0;
    };

    void execute(std::filesystem::path file_path);
}
