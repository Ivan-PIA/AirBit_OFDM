#include "../file_converter.h"
#include <iostream>

// g++ test_rand_bits.cpp ../file_converter.cpp -o test && ./test

int main() {

    auto bits = generateRandBits(100);
    for(auto &bit : bits) {
        std::cout << static_cast<int>(bit) << "";
    }
    std::cout << std::endl;
    return 0;
}