#include "fft.h"
#include <algorithm>
#include <numbers>
#include <omp.h> 
#include <iostream>

namespace
{
    using cd = std::complex<double>;
    const double PI = 3.141592653589793;
}

// Функция для получения ближайшей степени двойки
int nearest_power_of_two(int n) {
    if (n <= 0) return 1;
    int power = 1;
    while (power < n) {
        power <<= 1;
    }
    return power;
}

// Функция реверсирования
int reverse(int num, int lg_n) {
    int res = 0;
    for (int i = 0; i < lg_n; i++) {
        res |= (num & (1 << i)) ? (1 << (lg_n - 1 - i)) : 0;
    }
    return res;
}

// 

std::vector<cd> fft(const std::vector<cd>& num) {
    int n = num.size();
    int n_padded = nearest_power_of_two(n);
    std::vector<cd> fft_image(n_padded, 0); // Инициализация с нулями

    // Копируем исходные данные в начало
    std::copy(num.begin(), num.end(), fft_image.begin());

    int lg_n = log2(n_padded);

    // Реверсирование индексов
    #pragma omp parallel for
    for (int i = 0; i < n_padded; i++) {
        int rev_i = reverse(i, lg_n);
        if (i < rev_i) {
            std::swap(fft_image[i], fft_image[rev_i]);
        }
    }

    // Основной алгоритм FFT
    for (int len = 2; len <= n_padded; len <<= 1) {
        double ang = 2 * PI / len;
        cd wlen(cos(ang), sin(ang)); // Экспонента
        #pragma omp parallel for
        for (int i = 0; i < n_padded; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; j++) {
                cd u = fft_image[i + j];
                cd v = fft_image[i + j + len / 2] * w;
                fft_image[i + j] = u + v;
                fft_image[i + j + len / 2] = u - v;
                w *= wlen; // Умножение на wlen
            }
        }
    }

    return fft_image;
}


std::vector<cd> ifft(const std::vector<cd>& num) {
    std::vector<cd> ifft_image = fft(num);
    int n = ifft_image.size();

    for (cd& x : ifft_image)
        x /= n;

    std::reverse(ifft_image.begin() + 1, ifft_image.end());
    return ifft_image;
}

std::vector<cd> fftshift(const std::vector<cd> &num) {
    std::vector<cd> shifted(num.size());
    int n = num.size();
    int mid = (n + 1) / 2;

    for (int i = 0; i < n; i++) {
        shifted[i] = num[(i + mid) % n];
    }

    return shifted;
}
