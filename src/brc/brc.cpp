#include "brc.hpp"

#include <unordered_map>
#include <string>
#include <fstream>
#include <set>
#include <ranges>
#include <print>
#include <thread>


#include <absl/container/node_hash_map.h>
#include <parallel_hashmap/phmap.h>

// Sets byte of delimiter as 0b10000000
// Sets any other byte as    0b00000000
uint64_t brc::internal::findDelimiter(uint64_t word) {
    uint64_t input = word ^ 0x3B3B3B3B3B3B3B3BL;
    return (input - 0x0101010101010101L) & ~input & 0x8080808080808080L;
}

brc::Station& brc::internal::readHash(mio::mmap_source& mmap, brc::Result& result, size_t& ptr) {

    constexpr std::array<uint64_t, 9> mask = {
        0xFFL,                  //byte  0..=0
        0xFFFFL,                //bytes 0..=1
        0xFFFFFFL,              //bytes 0..=2
        0xFFFFFFFFL,            //bytes 0..=3
        0xFFFFFFFFFFL,          //bytes 0..=4
        0xFFFFFFFFFFFFL,        //bytes 0..=5
        0xFFFFFFFFFFFFFFL,      //bytes 0..=6
        0xFFFFFFFFFFFFFFFFL,    //bytes 0..=7
        0xFFFFFFFFFFFFFFFFL     //bytes 0..=7 (indicates no byte contains delimiter)
    };

    size_t name_start = ptr;

    // Read a 64 bit word from the mmap
    uint64_t word = *((uint64_t*)(&mmap[ptr]));

    // Sets byte of delimiter as 0b10000000
    // Sets any other byte as    0b00000000
    uint64_t word_1_delimiter_mask = findDelimiter(word);

    // std::countr_zero counts the number of bits before the 0
    // We shift by 3 to divide the result by 8 getting the associated byte
    // If no bit is set to 1, the result will be 8
    int word_1_delimiter_index = std::countr_zero((uint64_t)word_1_delimiter_mask) >> 3;

    // If the delimiter is not in the uint64_t advance 8 bytes
    // If the delimiter is in the uint64_t advance to delimiter
    ptr += word_1_delimiter_index;

    uint64_t hash = word & mask[word_1_delimiter_index];

    while (word_1_delimiter_index == 8) {
        word = *((uint64_t*)(&mmap[ptr]));
        word_1_delimiter_mask = findDelimiter(word);
        word_1_delimiter_index = std::countr_zero((uint64_t)word_1_delimiter_mask) >> 3;
        ptr += word_1_delimiter_index;

        // hash is constructed as an xor of the name bytes
        // this is usually a terrible idea as xor is communicative 
        // however, we are keeping the ; character which is known to not appear in station names
        hash ^= word & mask[word_1_delimiter_index];
    }

    // Bitshifts to help bits that will be masked will have useful data
    // Masks bits with size of vector to limit range to that off block vector
    uint32_t index = (uint32_t)((hash ^ (hash >> 33) ^ (hash >> 15)) & (brc::BLOCK_VECTOR_SIZE - 1));

    auto name_size = ptr - name_start;

    while (true) {
        brc::Station& element = result.block_vector[index];

        // Entires are zero initalized so we can garentee this is a new entry
        if (element.count == 0) {
            element.name_size = name_size;
            element.name_ptr = name_start;
            result.hashes.push_back(index);
            return element;
        }

        // Check for equality of names
        if (memcmp(&mmap[element.name_ptr], &mmap[name_start], name_size) == 0) {
            return element;
        }

        // Collisions detected, mixing index
        index = (index + 31) & (brc::BLOCK_VECTOR_SIZE - 1);
    }
}

size_t brc::internal::nextNewLine(mio::mmap_source& mmap, size_t ptr) {
    while (mmap[ptr++] != '\n') {}
    return ptr;
}

void brc::internal::processBlock(mio::mmap_source& mmap, brc::Result& result, size_t start, size_t len) {
    size_t block_end = nextNewLine(mmap, std::min(mmap.mapped_length() - 1, start + len));
    size_t block_start = start == 0 ? 0 : nextNewLine(mmap, start);

    size_t ptr = block_start;
    int16_t val;
    while (ptr < block_end - 1) {
        start = ptr;

        auto& element = readHash(mmap, result, ptr);
        ptr += 1;
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

        element.count += 1;
        element.total += val;

        if (val > element.max) {
            element.max = val;
        } 
        if (val < element.min) {
            element.min = val;
        }
    }
}

auto brc::execute(std::filesystem::path file_path) -> void {
    
    auto mmap = mio::mmap_source(file_path.string());
    auto size = mmap.mapped_length();

    auto block_size = size / THREAD_COUNT;

    auto threads = std::array<std::thread, THREAD_COUNT>();

    std::vector<Result> results{};
    results.resize(THREAD_COUNT);

    for (int i = 0; i < threads.size(); i++) {
        threads[i] = std::thread(internal::processBlock, std::ref(mmap), std::ref(results[i]), i * block_size, block_size);
    }

    auto stations = std::map<std::string_view, brc::Station>();

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
        
        for (auto& hash : results[i].hashes) {
            auto& hashed_element = results[i].block_vector[hash];
            auto& combined_element = stations[std::string_view(&mmap[hashed_element.name_ptr], hashed_element.name_size)];

            combined_element.count += hashed_element.count;
            combined_element.total += hashed_element.total;

            if (hashed_element.max > combined_element.max) {
                combined_element.max = hashed_element.max;
            } 
            if (hashed_element.min < combined_element.min) {
                combined_element.min = hashed_element.min;
            }
        }
    }

    std::print("{}", "{");
    for (auto&& [key, elem] : std::views::enumerate(stations)) {
        auto min = std::ceil(elem.second.min) / 10.0;
        auto average = std::ceil(((double) elem.second.total / elem.second.count)) / 10.0;
        auto max = std::ceil(elem.second.max) / 10.0;

        if (key != 0) {
            std::print("{}", ", ");
        }
        
        std::print("{}={:.1f}/{:.1f}/{:.1f}", elem.first, min, average, max);
    }
    std::println("{}", "}");

    mmap.unmap();
}