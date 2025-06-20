#include <iostream>
#include "../segmenter.h"

// g++ test_check_crc.cpp ../segmenter.cpp -o test && ./test

int main() {
    std::vector<uint8_t> bits = {
    0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,}; 
    Segmenter segmenter;
    
    std::vector<std::vector<uint8_t>> segments = segmenter.segment(bits, 0);
    
    std::cout << static_cast<int>(segments[0][100])<<std::endl;
    //segments[0][200] = 0; // за полезными данными и crc
    segments[0][100] = (segments[0][100] == 0) ? 1 : 0;
    
    auto err_crc = segmenter.checkCRC(segments);


    for (size_t i = 0; i < segments.size(); ++i) {
        std::cout << "\nСегмент " << i + 1 << ": \n";
        int bit_count = 0;
        for (const auto& bit : segments[i]) {
            std::cout << static_cast<int>(bit);
            bit_count++;
            if (bit_count % 8 == 0) {
                std::cout << ' ';
            }
        }
        std::cout << std::endl;
    }

    for (size_t i = 0; i < err_crc.size(); ++i) {
        std::cout << "\nОшибка в сегменте " << ": " << err_crc[i] << std::endl;
    }

    return 0;
}
