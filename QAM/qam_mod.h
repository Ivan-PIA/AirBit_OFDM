#ifndef QAM_MOD_H
#define QAM_MOD_H

#include <vector>
#include <complex>
#include <cmath>
#include <unordered_map>
#include <cstdint>
#include "../config.h"

using cd = std::complex<double>;

// Перечисление типов модуляции
enum ModulationType {
    BPSK,
    QPSK,
    QAM16,
    QAM64,
    None,
};

class QAM_mod {
public:
    // Модуляция на основе битовой матрицы и типа модуляции
    std::vector<std::vector<cd>> modulate(const std::vector<std::vector<uint8_t>>& bits, ModulationType modulationType = ModulationType::None);

private:
    std::vector<cd> symbols;
    
    // Установить символы на основе карты символов и битов
    void setSymbols(const std::vector<cd>& symbolMap, const std::vector<int>& bits, size_t symbolSize, double normalizationFactor);

    // Карты символов для различных типов модуляции
    static const std::unordered_map<ModulationType, std::tuple<std::vector<cd>, size_t, double>> modulationData;
};

#endif // QAM_MOD_H