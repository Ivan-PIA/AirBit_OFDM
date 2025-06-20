#include <iostream>
#include "../segmenter.h"

// g++ test.cpp ../segmenter.cpp -o test && ./test

int main() {
    std::vector<uint8_t> bits = {
    0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0, 1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,}; 
    Segmenter segmenter;
    
    std::vector<std::vector<uint8_t>> segments = segmenter.segment(bits);
    segments = segmenter.scramble(segments);
    segments = segmenter.scramble(segments);

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

    return 0;
}
