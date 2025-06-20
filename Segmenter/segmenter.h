#ifndef SEGMENTER_H
#define SEGMENTER_H

#include <vector>
#include <cstdint>
#include <bitset>
#include <random>

#include "../config.h"

class Segmenter {
public:
    Segmenter(
            uint32_t maxLenLine = MAX_LEN_LINE,
            uint32_t segmentNumBits = SEGMENT_NUM_BITS,
            uint32_t usefulBits = USEFUL_BITS,
            uint32_t crcBits = CRC_BITS,
            uint16_t flagBits = 2
            );

    std::vector<std::vector<uint8_t>> segment(const std::vector<uint8_t>& bits, uint8_t flag = 0);
    std::vector<std::vector<uint8_t>> scramble(const std::vector<std::vector<uint8_t>>& data);
    std::vector<std::vector<uint8_t>> reshape(const std::vector<uint8_t>& bits);
    std::vector<uint8_t> extract_data(const std::vector<std::vector<uint8_t>>& bits);
    uint8_t extract_flag(const std::vector<std::vector<uint8_t>>& bits);
    void get_size_data_in_slot() const;

    std::vector<int> checkCRC(const std::vector<std::vector<uint8_t>>& segments);

private:
    uint32_t maxLenLine;
    uint32_t segmentNumBits;
    uint32_t usefulBits;
    uint32_t crcBits;
    uint32_t dataBitsInput;
    uint32_t maxLenLineInSegment;
    uint32_t totalSegments;
    uint16_t flagBits;

    uint64_t computeCRC(const std::vector<uint8_t>& data);
};

#endif // SEGMENTER_H
