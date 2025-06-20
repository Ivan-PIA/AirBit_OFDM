#include "../file_converter.h"
#include <iostream>
#include <cassert>

// g++ test.cpp ../file_converter.cpp -o test && ./test

void printBits(const std::vector<uint8_t>& bits) {
    for (const auto& bit : bits) {
        std::cout << static_cast<int>(bit);
    }
    std::cout << std::endl;
}

int main() {
    const std::string originalFilePath = "./files_for_test/test.txt"; // Path to the original file
    const std::string outputDir = "./out_files"; // Directory to store the output file
    std::string fileName; // Will hold the file name

    try {
        // Convert file to bits with metadata
        std::vector<uint8_t> fileBits = file2bits(originalFilePath);

        // Output number of bits and the bits themselves
        std::cout << "Number of bits: " << fileBits.size() << std::endl;
        std::cout << "Bits: ";
        printBits(fileBits);

        // Convert bits back to file with metadata
        bits2file(outputDir, fileBits);

        std::cerr << "Test complete." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
    }

    return 0;
}
