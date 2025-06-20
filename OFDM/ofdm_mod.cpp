#include "ofdm_mod.h"
#include "fft/fft.h"
#include "sequence.h"

#include <iostream>

namespace {
    using cd = std::complex<double>;
}


OFDM_mod::OFDM_mod() 
{ 
    gen_pilots_siq(data.pilot_indices, data.refs);
    
    if (CP_LEN == 0) CP_len = N_FFT / 12.8;
    else CP_len = N_FFT / 4;
};


// OFDM модуляция
// На вход матрица IQ символов, на выходе сэмплы для передачи
std::vector<cd> OFDM_mod::modulate(const std::vector<std::vector<cd>> &input_matrix) {
    std::vector<cd> output;

    auto mapped_pss = mapPSS(0);
    // Резервируем место для циклического префикса и символа
    mapped_pss.insert(mapped_pss.begin(), mapped_pss.end() - CP_len, mapped_pss.end());

    uint16_t n_slot = 0;
    for (const auto &input_symbols : input_matrix) {
        // input_symbols - данные на 1 слот

        // Вставка PSS перед каждым слотом из 5 символов OFDM
        output.insert(output.end(), mapped_pss.begin(), mapped_pss.end());
        
        for (int k = 0; k < OFDM_SYM_IN_SLOT; ++k) {
            std::vector<cd> ofdm_symbol(input_symbols.begin() +  k      * (data.N_active_subcarriers-1)   + k,
                                        input_symbols.begin() + (k + 1) * (data.N_active_subcarriers-1)+1 + k); // +1 для включения последнего элемента

            ofdm_symbol = mapData(ofdm_symbol);
            ofdm_symbol = mapPilots(ofdm_symbol, n_slot, k);

            ofdm_symbol = fftshift(ofdm_symbol);

            auto time_domain_symbol = ifft(ofdm_symbol);

            // Добавление циклического префикса
            std::vector<cd> cp = addCyclicPrefix(time_domain_symbol, k);

            output.insert(output.end(), cp.begin(), cp.end());
        }
        n_slot++;

    }

    // Умножение на 2^10
    for (auto &sample : output) {
        sample *= std::pow(2, 14);
    }

    return output;
}

std::vector<cd> OFDM_mod::addCyclicPrefix(const std::vector<cd>& time_domain_symbol, int k) {
    std::vector<cd> cp;

    // Определение длины циклического префикса на основе индекса символа
    if (CP_LEN == 0) {
        cp.assign(time_domain_symbol.end() - CP_len * 0.9, time_domain_symbol.end());
    } else {
        cp.assign(time_domain_symbol.end() - CP_len, time_domain_symbol.end());
    }

    // Добавление циклического префикса к символу
    cp.insert(cp.end(), time_domain_symbol.begin(), time_domain_symbol.end());

    return cp;
}

void OFDM_Data::generateIndices() {
    data_indices.clear();
    data_indices_noPilots.clear();
    data_indices_shifted.clear();
    pilot_indices.clear();
    pilot_indices_shifted.clear();

    int left_active = G_SUBCAR / 2 + 1;
    int middle_subcarrier = N_FFT / 2;
    int total_active = (N_active_subcarriers + N_PILOTS + 1);
    int pilot_interval = total_active / (N_PILOTS - 1);
    int shift = 0;
    shift = shift % 6;

    // Проходим по всем поднесущим без защитной полосы
    for (int i = 0; i < total_active; ++i) {
        int current_subcarrier = left_active + i;

        // Пропускаем центральную поднесущую (DC)
        if (current_subcarrier == middle_subcarrier) {
            continue;
        }

        data_indices_noPilots.push_back(current_subcarrier);

        if ((i > (total_active - 1) / 2) && 
            (i % pilot_interval == (shift + 1) % 6) && 
            pilot_indices.size() < N_PILOTS) {
            pilot_indices.push_back(current_subcarrier);
        }
        // Расстановка пилотов в первой половине поднесущих
        else if ((i < (total_active - 1) / 2) && 
                 (i % pilot_interval == shift) && 
                 pilot_indices.size() < N_PILOTS) {
            pilot_indices.push_back(current_subcarrier);
        }
        // Остальные поднесущие используются для передачи данных
        else {
            data_indices.push_back(current_subcarrier);
        }
    }

    for (int i = 0; i < total_active; ++i) {
        int current_subcarrier = left_active + i;

        // Пропускаем центральную поднесущую (DC)
        if (current_subcarrier == middle_subcarrier) {
            continue;
        }

        if ((i > (total_active - 1) / 2) && 
            (i % pilot_interval == (shift + 1 + 3) % 6) && 
            pilot_indices_shifted.size() < N_PILOTS) {
            pilot_indices_shifted.push_back(current_subcarrier);
        }
        else if ((i < (total_active - 1) / 2) && 
                 (i % pilot_interval == (shift + 3) % 6) && 
                 pilot_indices_shifted.size() < N_PILOTS) {
            pilot_indices_shifted.push_back(current_subcarrier);
        }
        else {
            data_indices_shifted.push_back(current_subcarrier);
        }
    }
}



std::vector<cd> OFDM_mod::mapPilots(std::vector<cd> &input, uint16_t num_slot, uint16_t num_symbol) {

    std::vector<cd> pilots_val = data.refs[num_slot%20][num_symbol];
    // std::cout << "num_slot = " << num_slot << "  num_symbol = " << num_symbol <<  std::endl;
    // for(auto i : pilots_val) {
    //     std::cout << i << " ";
    // }
    // std::cout << std::endl;
    std::complex<double> p(1.0,1.0);
    uint16_t k = 0;
    for (int pilot_index : data.pilot_indices) {
        // input[pilot_index] = pilots_val[k]/sqrt(2); 
        input[pilot_index] = p /sqrt(2);
        // std::cout << "pilots_val[k]/sqrt(2) " << pilots_val[k]/sqrt(2) << " ";
        // std::cout << "pilot_index = " << pilot_index << std::endl;
        k++;
    }

    return input;
}


// Маппинг данных по поднесущим
std::vector<cd> OFDM_mod::mapData(const std::vector<cd> &input) {
    std::vector<cd> subcarriers(N_FFT, 0);

    // Расставляем данные по заранее известным индексам
    int data_index = 0;
    for (int data_pos : data.data_indices) {
        // std::cout <<  data_pos << " ";
        subcarriers[data_pos] = input[data_index++];
    }
    // printf("\n");
    
    return subcarriers;
}

// Маппинг PSS, ifft над PSS
// root(u) = 0 1 2
std::vector<cd> OFDM_mod::mapPSS(int u) {
    std::vector<cd> subcarriers(N_FFT, 0);
    std::vector<cd> pss;
    if (u == 0) {
        pss = ZadoffChu(25);
    } else if (u == 1) {
        pss = ZadoffChu(29);
    } else if (u == 2) {
        pss = ZadoffChu(34);
    }

    int left_active = (N_FFT / 2) - 31;
    
    uint16_t k = 0;
    for (int i = 0; i < 63; ++i) {
        if (i==31) subcarriers[left_active + i] = 0;
        else subcarriers[left_active + i] = pss[k++]; 
    }

    subcarriers = fftshift(subcarriers);

    subcarriers = ifft(subcarriers);

    return subcarriers;
}

// Маппинг PSS, ifft над PSS
std::vector<cd> OFDM_mod::mapSSS(int N_ID_cell) {
    std::vector<cd> subcarriers(N_FFT, 0);
    auto sss = generate_sss(N_ID_cell);

    int left_active = (N_FFT / 2) - 31;

    uint16_t k = 0;
    for (int i = 0; i < 63; ++i) {
        if (i==31) subcarriers[left_active + i] = 0;
        else subcarriers[left_active + i] = sss[k++]; 
    }

    subcarriers = fftshift(subcarriers);

    subcarriers = ifft(subcarriers);

    return subcarriers;
}

