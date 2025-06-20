#ifndef SEQUENSE_H
#define SEQUENSE_H

#include <vector>
#include <complex>
#include <cmath>
#include "../config.h"

using cd = std::complex<double>;

void gen_pilots_siq(const std::vector<int>& pilot_indices, std::vector<std::vector<std::vector<cd>>>& refs);
std::vector<cd> ZadoffChu(int u = 25);
std::vector<cd> generate_sss(int N_ID_cell);

#endif // SEQUENSE_H