#pragma once

#include <filesystem>
#include <limits>
#include <list>

#include <mio/mmap.hpp>

namespace brc {
    constexpr auto THREAD_COUNT = 8;
    constexpr int64_t BLOCK_VECTOR_SIZE = 1 << 17;
    constexpr size_t CHUNK_SIZE = 1 << 21;

    struct Station {
        int16_t min = 999;
        int16_t max = -999;
        uint32_t count = 0;
        int64_t total = 0;
    };

    struct Result {
        Station block_vector[BLOCK_VECTOR_SIZE] = {};
        std::string names[BLOCK_VECTOR_SIZE] = {};
        std::vector<int64_t> hashes = {};
    };

    void execute(std::filesystem::path file_path);

    namespace internal {
        void processBlock(mio::mmap_source& mmap, Result& result, std::atomic<size_t>& chunk_counter);
        size_t nextNewLine(mio::mmap_source& mmap, size_t ptr);
        uint32_t readHash(mio::mmap_source& mmap, Result& result, size_t& ptr, uint64_t word);
        uint64_t findDelimiter(uint64_t word);
    }
}
