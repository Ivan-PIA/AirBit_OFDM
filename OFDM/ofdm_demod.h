#ifndef OFDM_DEMOD_H
#define OFDM_DEMOD_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <omp.h>
#include <chrono>

#include "../config.h"

using cd = std::complex<double>;

class OFDM_demod {
public:
    OFDM_demod();

    std::vector<cd> demodulate(const std::vector<cd>& signal);

    std::vector<cd> convolve_fft(const std::vector<cd>& vec1, const std::vector<cd>& vec2);

    std::vector<double> correlation(const std::vector<std::complex<double>>& y1, const std::vector<std::complex<double>>& y2);
    
    std::vector<int> get_ind_pss(const std::vector<double> &corr, float threshold = 0.95);

private:
    int CP_len;

    std::vector<cd> demodulateSlot(const std::vector<cd>& signal, size_t n_slot, const std::vector<int>& indexs_pss, const std::vector<int>& data_indices);
    std::vector<cd> divideByChannel(const std::vector<cd>& one_symb_freq, const std::vector<cd>& inter_H);
    void displayProgress(size_t n_slot, size_t total_slots);

    //////////////

    std::vector<int> find_ind_pss(const std::vector<double> corr, float threshold = 0.95);
    std::vector<int> find_ind_cp(const std::vector<double>& corr_cp);

    std::vector<cd> extract_slots(const std::vector<cd>& signal, const std::vector<int>& indices, int n_slot);
    std::vector<cd> extract_symb (const std::vector<cd>& signal, const std::vector<int>& indices, int n_symb);

    std::vector<double> corr_cp(const std::vector<cd>& slot_signal);
    std::vector<cd> interpolated_H(const std::vector<cd>& signal, int n_slot, int n_symb);

    //////////////

    std::vector<int> find_ind_cp_normal(const std::vector<double>& corr_cp);
    std::vector<int> find_ind_cp_extended(const std::vector<double>& corr_cp);

    void sinr(const std::vector<cd>& signal, int first_ind_pss);
    double calculate_power(const std::vector<cd>& signal);
    
};


#endif // OFDM_MOD_H
