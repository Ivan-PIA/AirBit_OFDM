#include "qam_mod.h"
#include "qam_maps.h"
#include <iostream>


namespace {
    using cd = std::complex<double>;
}

// Карта символов для быстрого доступа
const std::unordered_map<ModulationType, std::tuple<std::vector<cd>, size_t, double>> QAM_mod::modulationData = {
    {BPSK, {BPSK_MAP, 1, 1.0}},
    {QPSK, {QPSK_MAP, 2, 2.0}},
    {QAM16, {QAM16_MAP, 4, 10.0}},
    {QAM64, {QAM64_MAP, 6, 42.0}},
};


// Установить символы на основе битов
void QAM_mod::setSymbols(const std::vector<cd>& symbolMap, const std::vector<int>& bits, size_t symbolSize, double normalizationFactor) {
    size_t numSymbols = bits.size() / symbolSize;
    symbols.resize(numSymbols); // Изменяем размер вектора символов

    for (size_t i = 0; i < numSymbols; ++i) {
        int index = 0;
        for (size_t j = 0; j < symbolSize; ++j) {
            index = (index << 1) | bits[i * symbolSize + j];
        }
        symbols[i] = symbolMap[index] / std::sqrt(normalizationFactor);
    }
}

// Модуляция на основе типа модуляции
std::vector<std::vector<cd>> QAM_mod::modulate(const std::vector<std::vector<uint8_t>>& bits, ModulationType modulationType) {
    // Если modulationType не задан, установить его на основе IQ_MODULATION
    if (modulationType == ModulationType::None) {
        switch (IQ_MODULATION) {
            case 1:
                modulationType = ModulationType::BPSK;
                break;
            case 2:
                modulationType = ModulationType::QPSK;
                break;
            case 4:
                modulationType = ModulationType::QAM16;
                break;
            case 6:
                modulationType = ModulationType::QAM64;
                break;
            default:
                std::cerr << "Unsupported IQ_MODULATION value" << std::endl;
                return {};
        }
    }

    // Проверка валидности modulationType
    if (modulationData.find(modulationType) == modulationData.end()) {
        std::cerr << "Unsupported modulation type" << std::endl;
        return {};
    }

    // Извлечение символной карты, размера символа и нормализующего коэффициента
    const auto& [symbolMap, symbolSize, normalizationFactor] = modulationData.at(modulationType);

    std::vector<std::vector<cd>> result;

    for (const auto& bitVec : bits) {
        std::vector<int> intBits(bitVec.begin(), bitVec.end());

        // Добавление нулей, если размер не кратен symbolSize
        while (intBits.size() % symbolSize != 0) {
            intBits.push_back(0);
        }

        // Установка символов и модуляция
        setSymbols(symbolMap, intBits, symbolSize, normalizationFactor);
        result.push_back(symbols);
    }

    return result;
}
