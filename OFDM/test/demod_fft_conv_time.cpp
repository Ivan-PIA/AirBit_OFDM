#include "../ofdm_demod.h"
#include "../ofdm_mod.h"
#include "../sequence.h"
#include "../fft/fft.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

// g++ demod_fft_conv_time.cpp ../ofdm_demod.cpp ../ofdm_mod.cpp ../fft/fft.cpp -o test && ./test

// НЕ РАБОТАЕТ
int main() {
    OFDM_demod OFDM_demod;

    // Создание массива из 1 миллиона значений
    std::vector<std::complex<double>> large_vec(30000);
    for (size_t i = 0; i < large_vec.size(); ++i) {
        large_vec[i] = {static_cast<double>(i % 100), static_cast<double>(i % 100)};
    }

    // Создание массива из 10 значений
    //std::vector<std::complex<double>> small_vec(16);
    OFDM_mod ofdm_mod;
    auto small_vec = ofdm_mod.mapPSS();
    for (size_t i = 0; i < small_vec.size(); ++i) {
        small_vec[i] = {static_cast<double>(i % 100), static_cast<double>(i % 100)};
    }

    // Тестирование функции convolve
    auto start_convolve = std::chrono::high_resolution_clock::now();
    auto convolved = OFDM_demod.convolve(large_vec, small_vec);
    auto end_convolve = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_convolve = end_convolve - start_convolve;

    // Тестирование функции correlate_with_shift
    auto start_correlate = std::chrono::high_resolution_clock::now();
    auto corr = OFDM_demod.correlateShifted(large_vec, small_vec, true);
    auto end_correlate = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_correlate = end_correlate - start_correlate;

    auto start_corr = std::chrono::high_resolution_clock::now();
    auto correlate_norm = OFDM_demod.correlate(large_vec, small_vec, false);
    auto end_corr = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_corre = end_corr - start_corr;

    auto start_cross_corr = std::chrono::high_resolution_clock::now();
    auto cross_correlate_norm = correlation(large_vec, small_vec);
    auto end_cross_corr = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_cross_corre = end_corr - start_corr;

    // Печать результатов
    std::cout << "Convolution took : " << duration_convolve.count() << " seconds." << std::endl;
    std::cout << "Correlation took : " << duration_correlate.count() << " seconds." << std::endl;
    std::cout << "Correlation norm : " << duration_corre.count() << " seconds." << std::endl;
    std::cout << "Cross corr  norm : " << duration_cross_corre.count() << " seconds." << std::endl;

    return 0;
}

// 500k
// Convolution took: 0.898063 seconds.
// Correlation took: 0.380306 seconds.

// Проведённые тесты:
// 1млн
// Convolution took: 1.83882 seconds.
// Correlation took: 0.743936 seconds.

// 10млн
// Convolution took: 33.6569 seconds.
// Correlation took: 8.63476 seconds.

// Convolution took : 43.0613 seconds.
// Correlation took : 66.0377 seconds.
// Correlation norm : 24.1171 seconds.

// 4 194 304 (2^22)
// Convolution took: 16.6048 seconds.
// Correlation took: 4.37574 seconds.


// 16 и 16
// Convolution took : 9.17e-05 seconds.
// Correlation took : 1.26e-05 seconds.
// Correlation norm : 6.5e-06 seconds.   COOL

// 16 и 16 * 128раз
// Convolution took : 0.0055006 seconds.
// Correlation took : 0.0003743 seconds.
// Correlation norm : 0.0002143 seconds.

// 16 и 16 * 10000раз
// Convolution took : 0.0845914 seconds.
// Correlation took : 0.0080698 seconds.
// Correlation norm : 0.0031859 seconds.