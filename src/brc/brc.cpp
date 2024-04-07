#include "brc.hpp"

#include <map>
#include <string>
#include <ranges>
#include <print>
#include <thread>
#include <atomic>

// Sets byte of delimiter as 0b10000000
// Sets any other byte as    0b00000000
uint64_t brc::internal::findDelimiter(uint64_t word) {
    uint64_t input = word ^ 0x3B3B3B3B3B3B3B3BL;
    return (input - 0x0101010101010101L) & ~input & 0x8080808080808080L;
}

uint32_t brc::internal::readHash(mio::mmap_source& mmap, brc::Result& result, size_t& ptr, uint64_t word) {

    constexpr std::array<uint64_t, 9> mask = {
        0xFFL,                  // byte  0..=0
        0xFFFFL,                // bytes 0..=1
        0xFFFFFFL,              // bytes 0..=2
        0xFFFFFFFFL,            // bytes 0..=3
        0xFFFFFFFFFFL,          // bytes 0..=4
        0xFFFFFFFFFFFFL,        // bytes 0..=5
        0xFFFFFFFFFFFFFFL,      // bytes 0..=6
        0xFFFFFFFFFFFFFFFFL,    // bytes 0..=7
        0xFFFFFFFFFFFFFFFFL     // bytes 0..=7 (indicates no byte contains delimiter)
    };

    size_t name_start = ptr;

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
        word = reinterpret_cast<uint64_t&>(mmap[ptr]);
        
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

    auto read_name = std::string_view(&mmap[name_start], ptr - name_start);

    while (true) {
        std::string& name = result.names[index];

        // Entires are zero initalized so we can garentee this is a new entry
        if (name.length() == 0) {
            name = read_name;
            result.hashes.push_back(index);
            return index;
        }

        // Check for equality of names
        if (name == read_name) {
            return index;
        }

        // Collisions detected, mixing index
        index = (index + 31) & (brc::BLOCK_VECTOR_SIZE - 1);
    }
}

size_t brc::internal::nextNewLine(mio::mmap_source& mmap, size_t ptr) {
    while (mmap[ptr++] != '\n') {}
    return ptr;
}

// Special method to convert a number in the ascii number into an int without branches created by Quan Anh Mai.
int64_t convertIntoNumber(int32_t decimalSepPos, int64_t numberWord) {
    int32_t shift = 28 - decimalSepPos;
    // signed is -1 if negative, 0 otherwise
    int64_t sign = (~numberWord << 59) >> 63;
    int64_t designMask = ~(sign & 0xFF);
    // Align the number to a specific position and transform the ascii to digit value
    int64_t digits = ((numberWord & designMask) << shift) & 0x0F000F0F00L;
    // Now digits is in the form 0xUU00TTHH00 (UU: units digit, TT: tens digit, HH: hundreds digit)
    // 0xUU00TTHH00 * (100 * 0x1000000 + 10 * 0x10000 + 1) =
    // 0x000000UU00TTHH00 + 0x00UU00TTHH000000 * 10 + 0xUU00TTHH00000000 * 100
    int64_t absValue = ((digits * 0x640a0001) >> 32) & 0x3FF;
    return (absValue ^ sign) - sign;
}

int64_t readValue(mio::mmap_source& mmap, size_t& ptr) {
    uint64_t number_word = reinterpret_cast<uint64_t&>(mmap[++ptr]);
    int32_t decimal_pos = std::countr_zero(~number_word & 0x10101000UL);
    ptr += (decimal_pos >> 3) + 3;
    return convertIntoNumber(decimal_pos, number_word);
}

void accumulateValue(brc::Station& station, int64_t value) {
    station.count += 1;
    station.total += value;

    if (value > station.max) {
        station.max = value;
    } 
    if (value < station.min) {
        station.min = value;
    }
}

void brc::internal::processBlock(mio::mmap_source& mmap, brc::Result& result, std::atomic<size_t>& chunk_counter) {
    while (true) {
        size_t start = chunk_counter.fetch_add(brc::CHUNK_SIZE);

        if (start > mmap.length()) {
            return;
        }

        size_t block_start = start == 0 ? 0 : nextNewLine(mmap, start);
        size_t block_end = nextNewLine(mmap, std::min(mmap.mapped_length() - 1, start + brc::CHUNK_SIZE)) - 1;

        size_t dist = (block_end - block_start) / 3;
        size_t boundry1 = nextNewLine(mmap, start + dist);
        size_t boundry2 = nextNewLine(mmap, start + dist * 2);

        size_t ptr = block_start;
        size_t ptr1 = boundry1;
        size_t ptr2 = boundry2;

        while (ptr < boundry1 && ptr1 < boundry2 && ptr2 < block_end) {
            uint64_t word = reinterpret_cast<uint64_t&>(mmap[ptr]);
            uint64_t word1 = reinterpret_cast<uint64_t&>(mmap[ptr1]);
            uint64_t word2 = reinterpret_cast<uint64_t&>(mmap[ptr2]);

            auto& element = result.block_vector[readHash(mmap, result, ptr, word)];
            auto& element1 = result.block_vector[readHash(mmap, result, ptr1, word1)];
            auto& element2 = result.block_vector[readHash(mmap, result, ptr2, word2)];

            int64_t val = readValue(mmap, ptr);
            int64_t val1 = readValue(mmap, ptr1);
            int64_t val2 = readValue(mmap, ptr2);

            accumulateValue(element, val);
            accumulateValue(element1, val1);
            accumulateValue(element2, val2);
        }

        while (ptr < boundry1) {
            uint64_t word = reinterpret_cast<uint64_t&>(mmap[ptr]);
            auto& element = result.block_vector[readHash(mmap, result, ptr, word)];
            int64_t val = readValue(mmap, ptr);
            accumulateValue(element, val);
        }

        while (ptr1 < boundry2) {
            uint64_t word = reinterpret_cast<uint64_t&>(mmap[ptr1]);
            auto& element = result.block_vector[readHash(mmap, result, ptr1, word)];
            int64_t val = readValue(mmap, ptr1);
            accumulateValue(element, val);
        }

        while (ptr2 < block_end) {
            uint64_t word = reinterpret_cast<uint64_t&>(mmap[ptr2]);
            auto& element = result.block_vector[readHash(mmap, result, ptr2, word)];
            int64_t val = readValue(mmap, ptr2);
            accumulateValue(element, val);
        }
    }
}

auto brc::execute(std::filesystem::path file_path) -> void {
    
    auto mmap = mio::mmap_source(file_path.string());

    auto threads = std::array<std::thread, THREAD_COUNT>();

    std::vector<Result> results{};
    results.resize(THREAD_COUNT);

    std::atomic<size_t> chunk_counter = 0;

    for (int i = 0; i < threads.size(); i++) {
        threads[i] = std::thread(internal::processBlock, std::ref(mmap), std::ref(results[i]), std::ref(chunk_counter));
    }

    auto stations = std::map<std::string_view, brc::Station>();

    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
        
        for (auto& hash : results[i].hashes) {
            auto& hashed_element = results[i].block_vector[hash];
            auto& combined_element = stations[results[i].names[hash]];

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