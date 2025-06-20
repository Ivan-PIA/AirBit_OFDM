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
#include "../../OFDM/fft/fft.h"
#include "../../File_converter/file_converter.h"
#include "../../OFDM/freq_offset.hpp"
#include "../../other/model_channel.h"
#include "../../other/plots.h"




// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test


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

std::vector<std::complex<double>> readComplexNumbersFromFile(const std::string& filename) {
    std::vector<std::complex<double>> result;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Remove unwanted characters
        line.erase(remove(line.begin(), line.end(), '('), line.end());
        line.erase(remove(line.begin(), line.end(), ')'), line.end());

        std::istringstream iss(line);
        double real, imag;
        char plus_or_minus, i;

        // Parse the real and imaginary parts
        if (iss >> real >> plus_or_minus >> imag >> i && (plus_or_minus == '+' || plus_or_minus == '-')) {
            if (plus_or_minus == '-') {
                imag = -imag;
            }
            result.emplace_back(real, imag);
        } else {
            throw std::runtime_error("Invalid format in file");
        }
    }

    file.close();
    return result;
}

std::vector<double> corr_cp_extended(const std::vector<cd>& slot_signal) {
    std::vector<double> corr(slot_signal.size(), 0.0);
    int CP_len = 9;
    OFDM_demod ofdm_demod;

    for (int i = 0; i <= slot_signal.size() - N_FFT - CP_len; ++i) {   
        std::vector<cd> first_win(slot_signal.begin() + i, slot_signal.begin() + i + CP_len);
        std::vector<cd> second_win(slot_signal.begin() + i + N_FFT, slot_signal.begin() + i + N_FFT + CP_len);
        
        std::vector<double> correlat = ofdm_demod.correlation(first_win, second_win);
        
        double correlat_cp = 0.0;
        for (auto var : correlat) correlat_cp += std::abs(var);
                
        corr[i] = correlat_cp;
    }

    return corr;
}

int main() { 
    // Чтение данных из файлов и создание вектора комплексных чисел
    //std::vector<cd> complexVector = readComplexVector("rx_imag_file.txt", "rx_real_file.txt");
    std::vector<cd> complexVector = readComplexNumbersFromFile("rx_file_complex_fs_1.92.txt");
    //complexVector = std::vector<std::complex<double>>(begin(complexVector), begin(complexVector) + 30000);
    //std::vector<cd> PSS_PBCH(begin(complexVector)+11000, begin(complexVector)+12500); // 4
 
    OFDM_mod ofdm_mod;
    OFDM_demod ofdm_demod;
    auto pss = ofdm_mod.mapPSS(1);
    auto corr = ofdm_demod.correlation(complexVector, pss);

    // std::vector<cd> PSS_PBCH_freq;
    // for(size_t i = 0; i < 1; i++){
    //     auto ff = fft(std::vector<cd>(begin(PSS_PBCH)+i*128, begin(PSS_PBCH)+(i+1)*128));
    //     PSS_PBCH_freq.insert(PSS_PBCH_freq.end(), begin(ff), end(ff));
    // }
    // cool_scatter(PSS_PBCH_freq, "PSS_PBCH_freq");

    // std::vector<cd> tes;
    // for(size_t i = 0; i < 400; i++){
    //     std::vector<cd> ff(begin(PSS_PBCH)+i, begin(PSS_PBCH)+i+128);
    //     tes.insert(tes.end(), begin(ff), end(ff));
    // }
    //spectrogram_plot(tes, 128, "tes");

    //auto corr = corr_cp_extended(complexVector);

    cool_plot(corr);
    // cool_plot(complexVector);
    spectrogram_plot(complexVector);



    show_plot();

    return 0;
}