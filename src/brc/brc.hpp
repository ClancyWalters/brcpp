#pragma once

#include <filesystem>
#include <limits>
#include <list>

#include <mio/mmap.hpp>

namespace brc {
    constexpr auto THREAD_COUNT = 8;
    constexpr int64_t BLOCK_VECTOR_SIZE = 1 << 17;

    struct Station {
        int16_t min = 999;
        int16_t max = -999;
        uint32_t count = 0;
        int64_t total = 0;
        uint32_t name_ptr = 0;
        uint32_t name_size = 0;
    };

    struct Result {
        Station block_vector[BLOCK_VECTOR_SIZE] = {};
        std::vector<int64_t> hashes = {};
    };

    void execute(std::filesystem::path file_path);

    namespace internal {
        void processBlock(mio::mmap_source& mmap, Result& result, size_t start, size_t len);
        size_t nextNewLine(mio::mmap_source& mmap, size_t ptr);
        Station& readHash(mio::mmap_source& mmap, Result& result, size_t& ptr);
        uint64_t findDelimiter(uint64_t word);
    }
}
