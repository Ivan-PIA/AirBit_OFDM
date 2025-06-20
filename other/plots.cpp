#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include <cmath>
#include "../OFDM/fft/fft.h"
#include "plots.h" 
#include "../QAM/qam_mod.h"
#include "../QAM/qam_demod.h"
#include "../OFDM/freq_offset.hpp"
#include "../OFDM/ofdm_demod.h"
#include "../OFDM/ofdm_mod.h"

namespace plt = matplotlibcpp;
using cd = std::complex<double>;


// Явное инстанцирование шаблона для unsigned char
template void cool_plot<unsigned char>(const std::vector<unsigned char>&, std::string, std::string, bool);

// Явная инстанциация для типа double
template void cool_plot<double>(const std::vector<double>&, std::string, std::string, bool);

// Первая версия функции: для одного вектора комплексных чисел
void cool_plot(const std::vector<cd>& data, std::string vid, std::string title, bool show_plot) {
    std::vector<double> real_part, imag_part;
    for (size_t i = 0; i < data.size(); ++i) {
        real_part.push_back(std::real(data[i]));
        imag_part.push_back(std::imag(data[i]));
    }
    double max_real = *std::max_element(real_part.begin(), real_part.end());
    double max_imag = *std::max_element(imag_part.begin(), imag_part.end());
    double min_real = *std::min_element(real_part.begin(), real_part.end());
    double min_imag = *std::min_element(imag_part.begin(), imag_part.end());
    double maximum = std::max(max_real, max_imag);
    double minimum = std::min(min_real, min_imag);
    plt::figure_size(1100, 600);
    plt::subplots_adjust({{"left", 0.07}, {"bottom", 0.08}, {"right", 0.95}, {"top", 0.92}});
    plt::title(title);
    plt::plot(real_part, vid);
    plt::plot(imag_part, vid);
    plt::grid(true);
    plt::xlim(-(data.size() * 0.02), data.size() * 1.02);
    plt::ylim(-(maximum * 1.15), maximum * 1.15);
    if (show_plot) {
        plt::show();
        plt::detail::_interpreter::kill();
    }
}

// Вторая версия функции: для одного вектора чисел (double)
template <typename T>
void cool_plot(const std::vector<T>& input, std::string title, std::string vid, bool show_plot) {
    std::vector<double> y;
    y.reserve(input.size());  // Зарезервировать память для нового вектора

    for (const T& value : input) {
        y.push_back(static_cast<double>(value));  // Преобразуем в double и добавляем в новый вектор
    }

    std::vector<double> x(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        x[i] = static_cast<double>(i);  // Используем индекс как ось X
    }

    double maximum = *std::max_element(y.begin(), y.end());
    double minimum = *std::min_element(y.begin(), y.end());

    plt::figure_size(1100, 600);
    plt::subplots_adjust({{"left", 0.07}, {"bottom", 0.08}, {"right", 0.95}, {"top", 0.92}});

    plt::title(title);
    plt::plot(x, y, vid);
    plt::grid(true);

    if (minimum >= 0) {
        plt::xlim(-(x.size() * 0.02), x.size() * 1.02);
        plt::ylim(-(maximum * 0.05), maximum * 1.05);
    } else {
        plt::xlim(-(x.size() * 0.02), x.size() * 1.02);
        plt::ylim(-(maximum * 1.05), maximum * 1.05);
    }

    if (show_plot) {
        plt::show();
        plt::detail::_interpreter::kill();
    }
}

void cool_scatter(const std::vector<cd>& data, const std::string title, bool show_plot) {
    // Prepare data for x and y
    std::vector<double> real_part, imag_part, maxi, colors;
    real_part.reserve(data.size());
    imag_part.reserve(data.size());
    int k = 0;
    for (const auto& val : data) {
        real_part.push_back(std::real(val)); // Real part for x
        imag_part.push_back(std::imag(val)); // Imaginary part for y
        maxi.push_back(std::abs(val));
        colors.push_back(static_cast<double>(k++) / data.size());
    }

    double maximum = *std::max_element(maxi.begin(), maxi.end());


    // Configure the scatter plot
    plt::figure_size(700, 700);
    plt::subplots_adjust({{"left", 0.07}, {"bottom", 0.05}, {"right", 0.94}, {"top", 0.94}});
    plt::title(title);
    plt::grid(true);
    plt::xlim(-(1.05 * maximum), 1.05 * maximum);
    plt::ylim(-(1.05 * maximum), 1.05 * maximum);
    plt::scatter_colored(real_part, imag_part, colors, 10.0, "hsv");

    // Add axis lines
    plt::axhline(0, 0, 1, {{"color", "black"}, {"linestyle", "--"}, {"linewidth", "1"}});
    plt::axvline(0, 0, 1, {{"color", "black"}, {"linestyle", "--"}, {"linewidth", "1"}});


    if (show_plot) {
        plt::show();
        plt::detail::_interpreter::kill();
    } 
}




void spectrogram_plot(const std::vector<cd>& input, const std::string& title, size_t FFT_Size, bool show_plot) {
    if (input.size() < FFT_Size) {
        throw std::invalid_argument("Input size must be greater than or equal to FFT_Size.");
    }

    size_t num_blocks = input.size() / FFT_Size;  // Число блоков
    std::vector<std::vector<float>> spectrogram(num_blocks, std::vector<float>(FFT_Size, 0.0f));

    // Разбиваем входной сигнал на блоки и вычисляем FFT
    for (size_t block = 0; block < num_blocks; ++block) {
        std::vector<cd> block_data(input.begin() + block * FFT_Size, input.begin() + (block + 1) * FFT_Size);

        // Выполняем FFT (реализуйте свою FFT функцию или используйте библиотеку)
        auto fft_result = fft(block_data);
        fft_result = fftshift(fft_result);

        // Считаем амплитуды спектра
        for (size_t k = 0; k <= FFT_Size; ++k) {  // Только положительные частоты
            spectrogram[block][k] = static_cast<float>(std::abs(fft_result[k]));
        }
    }

    // Транспонируем спектрограмму
    std::vector<std::vector<float>> transposed_spectrogram(FFT_Size, std::vector<float>(num_blocks, 0.0f));
    for (size_t i = 0; i < num_blocks; ++i) {
        for (size_t j = 0; j < FFT_Size; ++j) {
            transposed_spectrogram[j][i] = spectrogram[i][j];
        }
    }

    // Преобразуем транспонированную спектрограмму в одномерный массив для imshow
    std::vector<float> flattened_spectrogram;
    flattened_spectrogram.reserve(FFT_Size * num_blocks);  // Предварительно резервируем память
    for (const auto& row : transposed_spectrogram) {
        flattened_spectrogram.insert(flattened_spectrogram.end(), row.begin(), row.end());
    }

    // Проверка на правильность размера
    if (flattened_spectrogram.size() != FFT_Size * num_blocks) {
        std::cerr << "Flattened spectrogram size mismatch!" << std::endl;
    }

    // Построение спектрограммы
    plt::figure_size(1200, 600);  // Устанавливаем размер фигуры
    plt::title(title);

    // Рисуем спектрограмму
    plt::imshow(flattened_spectrogram.data(), 
                static_cast<int>(FFT_Size),  // Теперь строки — это частоты
                static_cast<int>(num_blocks),  // А столбцы — временные блоки
                1, 
                {{"aspect", "auto"}, {"cmap", "jet"}});  // Aspect ratio и цветовая карта
    plt::xlabel("Symbols");
    plt::ylabel("Subcarriers");



    if (show_plot) {
        plt::pause(0.1);  // Пауза для обновления графика
        plt::draw();      // Обновление графика
    }
}


void show_plot() {
    plt::show();
    plt::detail::_interpreter::kill();
}


inline void remove_cyclic_prefix(
    const std::vector<std::complex<double>>& rx_data,
    std::vector<std::complex<double>>& output,
    size_t N_cp,
    size_t N_fft)
{
    size_t N_full = N_fft + N_cp; // Полезная часть символа
    size_t num_symbols = rx_data.size() / N_full; // Количество OFDM-символов

    output.clear();
    output.reserve(num_symbols * N_fft); // Оптимизация памяти

    for (size_t i = 0; i < num_symbols; ++i)
    {
        size_t start = i * N_full + N_cp; // Начинаем после CP
        output.insert(output.end(), rx_data.begin() + start, rx_data.begin() + start + N_fft);
    }
}


std::vector<std::complex<double>> demod_qam(std::vector<std::complex<double>> &rx_data, size_t FFT_Size) {
    std::vector<std::complex<double>> demodulated_symbols;
    
    // Проверяем, что входные данные корректны
    if (rx_data.size() < FFT_Size) {
        std::cerr << "Error: rx_data size is smaller than FFT_Size!" << std::endl;
        return demodulated_symbols;
    }

    size_t num_ofdm_symbols = rx_data.size() / FFT_Size;
    std::vector<std::vector<std::complex<double>>> ofdm_symbols;

    // Разделяем входной поток на OFDM-символы, пропуская каждый 6-й символ (PSS)
    for (size_t i = 0; i < num_ofdm_symbols; ++i) {
        if ((i + 1) % 6 == 0) {  // Каждый 6-й символ — это PSS, его пропускаем
            continue;
        }

        std::vector<std::complex<double>> symbol(rx_data.begin() + i * FFT_Size,
                                                 rx_data.begin() + (i + 1) * FFT_Size);
        ofdm_symbols.push_back(symbol);
    }

    // Применяем FFT к каждому OFDM-символу
    for (auto& symbol : ofdm_symbols) {
        auto fft_result = fft(symbol);  // Применяем FFT
        fft_result = fftshift(fft_result);  // Выполняем fftshift для упорядочивания спектра

        // Добавляем демодулированные символы в выходной массив
        demodulated_symbols.insert(demodulated_symbols.end(), fft_result.begin(), fft_result.end());
    }

    return demodulated_symbols;
}


std::vector<std::complex<double>> demod_ofdm_qam(std::vector<std::complex<double>> &rx_data,
                                                 size_t FFT_Size) {
    std::vector<std::complex<double>> demodulated_symbols;
    std::vector<std::complex<double>> demodulated_symbols_no_equal;
    if (rx_data.size() < FFT_Size) {
        std::cerr << "Ошибка: rx_data меньше FFT_Size!" << std::endl;
        return demodulated_symbols;
    }

    OFDM_mod ofdm_mod;
    const OFDM_Data& data = ofdm_mod.getData();
    std::vector<int> data_indices1 = data.data_indices_shifted;
    

    std::vector<int>  pilot_indices = data.pilot_indices;


    size_t num_ofdm_symbols = rx_data.size() / FFT_Size;
    std::vector<std::vector<std::complex<double>>> ofdm_symbols;

    // Разбиваем данные на OFDM-символы, пропуская PSS (каждый 6-й символ)
    for (size_t i = 0; i < num_ofdm_symbols; ++i) {
        if ((i + 1) % 6 == 0) continue; // Пропускаем PSS

        std::vector<std::complex<double>> symbol(rx_data.begin() + i * FFT_Size,
                                                 rx_data.begin() + (i + 1) * FFT_Size);

                                                         
        double max_val = 0.0;
        for (const auto& sym : symbol) {
            max_val = std::max(max_val, std::abs(sym));  // Находим максимум
        }

        if (max_val > 0) {  
            for (auto& sym : symbol) {
                sym /= max_val;  
            }
        }

        
        ofdm_symbols.push_back(symbol);
    }

    std::complex<double> pilot(1,1);
    // pilot = pilot / sqrt(2);

    // Демодулируем каждый OFDM-символ
    for (auto& symbol : ofdm_symbols) {
        auto fft_result = fft(symbol);
        fft_result = fftshift(fft_result);



        std::vector<std::complex<double>> channel_estimate(FFT_Size);
        for (size_t i = 0; i < pilot_indices.size(); ++i) {
            size_t index = pilot_indices[i];
            // std::cout << "index= " << index << std::endl;
            channel_estimate[index] = fft_result[index] / pilot; // H(f) = Y(f) / X(f)
        }


        for (size_t i = 1; i < pilot_indices.size(); ++i) {
            size_t prev = pilot_indices[i - 1];
            size_t next = pilot_indices[i];
            std::complex<double> h_prev = channel_estimate[prev];
            std::complex<double> h_next = channel_estimate[next];

            for (size_t j = prev + 1; j < next; ++j) {
                double alpha = static_cast<double>(j - prev) / (next - prev);
                channel_estimate[j] = (1 - alpha) * h_prev + alpha * h_next;  // Линейная интерполяция
            }
        }


        for (size_t i = 0; i < FFT_Size; ++i) {
            if (channel_estimate[i] != std::complex<double>(0, 0)) { // Избегаем деления на 0
                fft_result[i] = fft_result[i] / channel_estimate[i];
            }
        }




        for (size_t i = 0; i < data_indices1.size(); ++i) {
            // std::cout << data_indices[i] << " ";
            if(std::abs(fft_result[data_indices1[i]]) <= 2.2)    
                demodulated_symbols.push_back(fft_result[data_indices1[i]]);
        }
        // printf("\n");
        // 
        demodulated_symbols_no_equal.insert(demodulated_symbols_no_equal.end(), fft_result.begin(), fft_result.end());
    }

    return demodulated_symbols;
}

void spectrogram_plot_subplot(const std::vector<std::complex<double>>& input, const std::string& title, size_t FFT_Size, bool show_plot) {

    if (input.size() < FFT_Size) {
        throw std::invalid_argument("Input size must be greater than or equal to FFT_Size.");
    }
    std::vector<std::complex<double>> rx_data = input;

    std::complex<double> j(0, 1);
    std::vector<std::complex<double>> off_rx_data(rx_data.size());
    // for(int i = 0; i < off_rx_data.size(); i++){
    //     off_rx_data[i] = rx_data[i] * std::exp(-j * 2.0 * M_PI * 3200.0 * ((double)i/(double)FS_2));
    // }
    // std::complex<double> result = std::exp(-j * 2.0 * M_PI * 2000 * t);

    OFDM_mod ofdm_mod;
    auto pss = ofdm_mod.mapPSS(0); 

    OFDM_demod ofdm_demod;
    std::cout << "size rx " << rx_data.size() <<std::endl;
    auto corr_pss = ofdm_demod.correlation(rx_data, pss);
    std::cout << "size cor: " << corr_pss.size() << std::endl;
    auto indexs_pss = ofdm_demod.get_ind_pss(corr_pss, 0.80); 
    // std::cout << "size ind: " << indexs_pss.size() << std::endl;
    if(indexs_pss.size() < 1){
        std::cout << "Not PSS: "<< std::endl;
        return;
    }
    int ind_first = indexs_pss[0];
    
    std::cout << "indexs_pss[0].plot " << indexs_pss[0]<<std::endl;
    rx_data.assign(rx_data.begin() + ind_first+N_FFT, rx_data.end());

    // std::vector<cd> data_cfo;
    // std::vector<std::complex<double>> processed_data_off;
    std::vector<std::complex<double>> processed_data;
    remove_cyclic_prefix(rx_data, processed_data, 32, 128);
    // remove_cyclic_prefix(off_rx_data, processed_data_off, 32, 128);

    // frequency_correlation(pss, processed_data_off, FS_2/N_FFT, data_cfo, FS_2);
    
    

    // std::vector<cd> data_cfo;
    // frequency_correlation(pss, processed_data, FS_2/N_FFT, data_cfo, FS_2);

    size_t num_blocks  = input.size()          / FFT_Size;
    size_t num_blocks2 = processed_data.size() / FFT_Size;

    std::vector<std::vector<float>> spectrogram(num_blocks, std::vector<float>(FFT_Size, 0.0f));
    std::vector<std::vector<float>> spectrogram2(num_blocks2, std::vector<float>(FFT_Size, 0.0f));

    for (size_t block = 0; block < num_blocks; ++block) {
        std::vector<std::complex<double>> block_data(input.begin() + block * FFT_Size, input.begin() + (block + 1) * FFT_Size);
        auto fft_result = fft(block_data);
        fft_result = fftshift(fft_result);

        for (size_t k = 0; k < FFT_Size; ++k) {
            spectrogram[block][k] = static_cast<float>(std::abs(fft_result[k]));
        }
    }
    for (size_t block = 0; block < num_blocks2; ++block) {
        std::vector<std::complex<double>> block_data2(processed_data.begin() + block * FFT_Size, processed_data.begin() + (block + 1) * FFT_Size);
        auto fft_result2 = fft(block_data2);
        fft_result2 = fftshift(fft_result2);

        for (size_t k = 0; k < FFT_Size; ++k) {
            spectrogram2[block][k] = static_cast<float>(std::abs(fft_result2[k]));
        }
    }

    std::vector<std::vector<float>> transposed_spectrogram(FFT_Size, std::vector<float>(num_blocks, 0.0f));
    for (size_t i = 0; i < num_blocks; ++i) {
        for (size_t j = 0; j < FFT_Size; ++j) {
            transposed_spectrogram[j][i] = spectrogram[i][j];
        }
    }

    std::vector<std::vector<float>> transposed_spectrogram2(FFT_Size, std::vector<float>(num_blocks2, 0.0f));
    for (size_t i = 0; i < num_blocks2; ++i) {
        for (size_t j = 0; j < FFT_Size; ++j) {
            transposed_spectrogram2[j][i] = spectrogram2[i][j];
        }
    }


    std::vector<float> flattened_spectrogram;
    flattened_spectrogram.reserve(FFT_Size * num_blocks);
    for (const auto& row : transposed_spectrogram) {
        flattened_spectrogram.insert(flattened_spectrogram.end(), row.begin(), row.end());
    }

    std::vector<float> flattened_spectrogram2;
    flattened_spectrogram2.reserve(FFT_Size * num_blocks2);
    for (const auto& row : transposed_spectrogram2) {
        flattened_spectrogram2.insert(flattened_spectrogram2.end(), row.begin(), row.end());
    }
    

    // auto qam_sym = demod_qam(processed_data, 128);

    auto qam_sym = demod_ofdm_qam(processed_data, 128);

    // auto qam_sym2 = demod_ofdm_qam(processed_data, 128);



    std::vector<double> real_part, imag_part;
    real_part.reserve(input.size());
    imag_part.reserve(input.size());

    for (const auto& val : input) {
        real_part.push_back(std::real(val));
        imag_part.push_back(std::imag(val));
    }

    std::vector<double> real_part_qam, imag_part_qam;
    real_part_qam.reserve(qam_sym.size());
    imag_part_qam.reserve(qam_sym.size());

    for (const auto& val1 : qam_sym) {
        real_part_qam.push_back(std::real(val1));
        imag_part_qam.push_back(std::imag(val1));
    }
    
    
    // std::vector<double> real_part_qam2, imag_part_qam2;
    // real_part_qam2.reserve(qam_sym2.size());
    // imag_part_qam2.reserve(qam_sym2.size());

    // for (const auto& val1 : qam_sym2) {
    //     real_part_qam2.push_back(std::real(val1));
    //     imag_part_qam2.push_back(std::imag(val1));
    // }



    static bool first_plot = true;

    if (show_plot) {
        if (first_plot) {
            plt::ion();
            plt::figure_size(1080, 1080);
            // plt::figure();
            first_plot = false;
        }

        plt::clf();  

        // Первый подграфик - спектрограмма
        plt::subplot(2, 2, 1);
        plt::imshow(flattened_spectrogram.data(),
                    static_cast<int>(FFT_Size),
                    static_cast<int>(num_blocks),
                    1,
                    {{"aspect", "auto"}, {"cmap", "jet"}});
        plt::title(title);
        plt::xlabel("Symbols");
        plt::ylabel("Subcarriers");

        // Второй подграфик - Реальная и мнимая часть
        plt::subplot(2, 2, 2);
        plt::plot(real_part); 
        plt::plot(imag_part);
        plt::title("Real vs Imaginary Part");
        plt::xlabel("Real Part");
        plt::ylabel("Imaginary Part");

        plt::subplot(2, 2, 3);
        plt::imshow(flattened_spectrogram2.data(),
                    static_cast<int>(FFT_Size),
                    static_cast<int>(num_blocks2),
                    1,
                    {{"aspect", "auto"}, {"cmap", "jet"}});
        
        plt::xlabel("Symbols");
        plt::ylabel("Subcarriers");

        plt::subplot(2, 2, 4);
        plt::scatter(real_part_qam, imag_part_qam, 10);
        plt::title("QAM");

        // plt::subplot(3, 2, 5);
        // plt::scatter(real_part_qam2, imag_part_qam2, 10);
        // plt::title("QAM");


        plt::pause(0.1);
        plt::draw();
    }
}



void spectrogram_plot_work(const std::vector<std::complex<double>>& input, const std::string& title, size_t FFT_Size, bool show_plot) {

    if (input.size() < FFT_Size) {
        throw std::invalid_argument("Input size must be greater than or equal to FFT_Size.");
    }

    size_t num_blocks = input.size() / FFT_Size;  // Число блоков
    std::vector<std::vector<float>> spectrogram(num_blocks, std::vector<float>(FFT_Size, 0.0f));

    // Разбиваем входной сигнал на блоки и вычисляем FFT
    for (size_t block = 0; block < num_blocks; ++block) {
        std::vector<cd> block_data(input.begin() + block * FFT_Size, input.begin() + (block + 1) * FFT_Size);

        // Выполняем FFT (реализуйте свою FFT функцию или используйте библиотеку)
        auto fft_result = fft(block_data);
        fft_result = fftshift(fft_result);

        // Считаем амплитуды спектра
        for (size_t k = 0; k < FFT_Size; ++k) {  // Только положительные частоты
            spectrogram[block][k] = static_cast<float>(std::abs(fft_result[k]));
        }
    }

    // Транспонируем спектрограмму
    std::vector<std::vector<float>> transposed_spectrogram(FFT_Size, std::vector<float>(num_blocks, 0.0f));
    for (size_t i = 0; i < num_blocks; ++i) {
        for (size_t j = 0; j < FFT_Size; ++j) {
            transposed_spectrogram[j][i] = spectrogram[i][j];
        }
    }

    std::vector<float> flattened_spectrogram;
    flattened_spectrogram.reserve(FFT_Size * num_blocks);
    for (const auto& row : transposed_spectrogram) {
        flattened_spectrogram.insert(flattened_spectrogram.end(), row.begin(), row.end());
    }

    static bool first_plot = true;

    if (show_plot) {
        if (first_plot) {
            plt::ion(); 
            plt::figure_size(600, 600);
            first_plot = false;
        }

        plt::clf();  

        plt::title(title);
        plt::imshow(flattened_spectrogram.data(), 
                    static_cast<int>(FFT_Size),  
                    static_cast<int>(num_blocks),  
                    1, 
                    {{"aspect", "auto"}, {"cmap", "jet"}});

        plt::xlabel("Symbols");
        plt::ylabel("Subcarriers");

        plt::pause(0.1);  
        plt::draw();      
    }
}

