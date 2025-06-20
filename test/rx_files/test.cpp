#include <iostream>
#include <fstream>
#include <vector>
#include <complex>
#include <string>

#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../OFDM/ofdm_demod.h"
#include "../../OFDM/sequence.h"
#include "../../File_converter/file_converter.h"



// g++ test.cpp  ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp -o test && ./test

using cd = std::complex<double>;

std::vector<std::complex<double>> readComplexVector(const std::string& realFile, const std::string& imagFile) {
    std::ifstream realStream(realFile);
    std::ifstream imagStream(imagFile);

    std::vector<std::complex<double>> complexVector;
    
    if (!realStream.is_open() || !imagStream.is_open()) {
        std::cerr << "Не удалось открыть файлы." << std::endl;
        return complexVector;
    }
    
    double realValue, imagValue;
    while (realStream >> realValue && imagStream >> imagValue) {
        complexVector.emplace_back(realValue, imagValue);
    }

    return complexVector;
}

void saveComplexVector(const std::vector<std::complex<double>>& complexVector, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : complexVector) {
        outFile << value.real() << " " << value.imag() << "\n";
    }
}

void saveCorr(const std::vector<double>& corr, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
        return;
    }

    for (const auto& value : corr) {
        outFile << value << "\n";
    }
}

int main() { 
    std::string realFile = "v_real.txt";
    std::string imagFile = "v_imag.txt";
    std::string outputFile = "out.txt";

    // Чтение данных из файлов и создание вектора комплексных чисел
    std::vector<cd> complexVector = readComplexVector(realFile, imagFile);
    complexVector = std::vector<std::complex<double>>(begin(complexVector), begin(complexVector) + 23400);

    OFDM_mod ofdm_mod;
    OFDM_demod ofdm_demod;
    auto pss = ofdm_mod.mapPSS(0);
    //auto corr = ofdm_demod.convolve(complexVector, pss);
    //auto corr = ofdm_demod.correlate(complexVector, pss);
    auto corr = correlation(complexVector, pss);

    // Сохранение вектора комплексных чисел в файл
    //saveComplexVector(complexVector, outputFile);
    saveCorr(corr, outputFile);

    return 0;
}