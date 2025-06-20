#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <iomanip>
#include <omp.h>

using namespace std;

#define M_PI 3.14159265358979323846


void frequency_correlation(const std::vector<std::complex<double>>& pss, const std::vector<std::complex<double>>& matrix_name, \
double m, std::vector<std::complex<double>>& data_offset, int fs);
