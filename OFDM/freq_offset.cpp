#include "freq_offset.hpp"


#define M_PI 3.14159265358979323846

template <typename T>

size_t argmax(std::vector<T> & v) {
    size_t max_index = 0;
    T max_value = v[0];
    for (size_t i = 1; i < v.size(); i++) {
        if (v[i] > max_value) {
            max_index = i;
            max_value = v[i];
        }
    }
    return max_index;
}


void frequency_correlation(const std::vector<std::complex<double>>& pss, const std::vector<std::complex<double>>& matrix_name, double m, std::vector<std::complex<double>>& data_offset, int fs) {

    vector<std::complex<double>> corr_coef(pss.rbegin(), pss.rend());
    #pragma omp parallel for
    for (size_t i = 0; i < corr_coef.size(); i++) {
        corr_coef[i] = conj(corr_coef[i]);
    }

    vector<std::complex<double>> partA(pss.size() + matrix_name.size() - 1, 0);
    #pragma omp parallel for
    for (size_t i = 0; i < pss.size() / 2; i++) {
        for (size_t j = 0; j < matrix_name.size(); j++) {
            partA[i + j] += corr_coef[i] * matrix_name[j];
        }
    }

    vector<std::complex<double>> xDelayed(matrix_name.size());
    #pragma omp parallel for
    for (size_t i = 0; i < matrix_name.size() ; i++) {
        
        if(i < pss.size() / 2){
            xDelayed[i] = 0;
        }
        else{
            xDelayed[i] = matrix_name[i];
        }
    }

    
    vector<std::complex<double>> partB(pss.size() + matrix_name.size() - 1, 0);
    #pragma omp parallel for
    for (size_t i = pss.size() / 2; i < pss.size(); i++) {
        for (size_t j = 0; j < xDelayed.size(); j++) {
            partB[i + j] += corr_coef[i] * xDelayed[j];
        }
    }


    vector<double> correlation(pss.size() + matrix_name.size() - 1, 0);
    #pragma omp parallel for
    for (size_t i = 0; i < correlation.size(); i++) {
        correlation[i] = abs(partA[i] + partB[i]);
    }
    vector<std::complex<double>> phaseDiff(pss.size() + matrix_name.size() - 1, 0);
    #pragma omp parallel for
    for (size_t i = 0; i < phaseDiff.size(); i++) {
        phaseDiff[i] = partA[i] * conj(partB[i]);
    }


    size_t istart = argmax(correlation) ;
    std::complex<double> phaseDiff_max = phaseDiff[istart];


    double CFO = arg(phaseDiff_max) / (M_PI * 1 / m);
    vector<double> t(matrix_name.size());
    #pragma omp parallel for
    for (size_t i = 0; i < t.size(); i++) {
        t[i] = i ;
    }

    cout << "CFO :" << CFO << endl;

    data_offset.resize(matrix_name.size());
    #pragma omp parallel for
    for (size_t i = 0; i < matrix_name.size(); i++) {
        
        data_offset[i] = matrix_name[i] * exp(-1i * double(2) * M_PI * CFO * double(t[i]/fs));
    }
    
}


