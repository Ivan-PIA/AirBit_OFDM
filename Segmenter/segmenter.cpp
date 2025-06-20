#include "segmenter.h"
#include <iostream>


// Конструктор класса Segmenter
Segmenter::Segmenter(
    uint32_t maxLenLine,
    uint32_t segmentNumBits,
    uint32_t usefulBits,
    uint32_t crcBits,
    uint16_t flagBits)
    : maxLenLine(maxLenLine), segmentNumBits(segmentNumBits),
      usefulBits(usefulBits), crcBits(crcBits), flagBits(flagBits) {}

// Функция разбиения вектора бит на сегменты
// Flag: 0 случайные биты, 1 - текст, 2 - файл
std::vector<std::vector<uint8_t>> Segmenter::segment(const std::vector<uint8_t>& bits, uint8_t flag) {
    std::vector<std::vector<uint8_t>> segments;
    setlocale(LC_ALL, "Russian");
    dataBitsInput = bits.size();
    maxLenLineInSegment = maxLenLine - segmentNumBits - usefulBits - crcBits - flagBits;
    totalSegments = (dataBitsInput + maxLenLineInSegment - 1) / maxLenLineInSegment;

    //DEBUG
    // std::cout << "Всего бит: " << bits.size()
    // << "\nОбщее количество сегментов: " << totalSegments
    // << "\nМаксимальное количество полезных бит в сегменте: " << maxLenLineInSegment << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dis(0, 1);

    for (uint32_t i = 0; i < totalSegments; ++i) {
        std::vector<uint8_t> segment;

        // Номер сегмента
        for (int j = segmentNumBits - 1; j >= 0; --j) {
            segment.push_back((i >> j) & 1);
        }

        // Добавление флага (2 бита)
        for (int j = flagBits-1; j >= 0; --j) {
            segment.push_back((flag >> j) & 1);
        }

        // Длина полезных бит
        uint32_t usefulBitsLength = (i == totalSegments - 1) ? (bits.size() % maxLenLineInSegment) : maxLenLineInSegment;
        if (usefulBitsLength == 0) usefulBitsLength = maxLenLineInSegment; // Когда bits.size() является кратным maxLenLineInSegment

        for (int j = usefulBits - 1; j >= 0; --j) {
            segment.push_back((usefulBitsLength >> j) & 1);
        }

        // Полезные данные
        uint32_t start = i * maxLenLineInSegment;
        for (uint32_t j = 0; j < usefulBitsLength; ++j) {
            segment.push_back(bits[start + j]);
        }

        // Вычисление CRC
        uint64_t crc = computeCRC(segment);
        for (int j = crcBits - 1; j >= 0; --j) {
            segment.push_back((crc >> j) & 1);
        }

        // Добавление случайных битов, если не хватает до maxLenLine
        uint32_t remainingBits = maxLenLine - segment.size();
        for (uint32_t j = 0; j < remainingBits; ++j) {
            segment.push_back(dis(gen));
        }

        segments.push_back(segment);
    }

    return segments;
}

// Функция вычисления CRC
uint64_t Segmenter::computeCRC(const std::vector<uint8_t>& data) {
    const uint64_t polynom = 0xA745; // 1010 0111 0100 0101
    uint64_t crc = (1ULL << crcBits) - 1; // Инициализация CRC с учетом заданного количества бит

    for (auto bit : data) {
        bool xor_flag = crc & (1ULL << (crcBits - 1));
        crc = ((crc << 1) | bit) & ((1ULL << crcBits) - 1); // Обрезаем CRC до нужного количества бит
        if (xor_flag) {
            crc ^= polynom;
        }
    }

    return crc & ((1ULL << crcBits) - 1); // Возвращаем CRC, обрезанный до нужного количества бит
}

// Функция проверки CRC
std::vector<int> Segmenter::checkCRC(const std::vector<std::vector<uint8_t>>& segments) {
    std::vector<int> incorrectSegments;

    for (size_t i = 0; i < segments.size(); ++i) {
        const auto& segment = segments[i];

        // Номер сегмента (первые segmentNumBits бит)
        int segmentIndex = 0;
        for (int j = 0; j < segmentNumBits; ++j) {
            segmentIndex = (segmentIndex << 1) | segment[j];
        }
        if (i % static_cast<int>(pow(2.0, double(segmentNumBits))) != segmentIndex) {
            incorrectSegments.push_back(i);
            continue;
        }

        // Длина полезных бит (следующие usefulBits бит)
        int usefulBitsLength = 0;
        for (int j = 0; j < usefulBits; ++j) {
            usefulBitsLength = (usefulBitsLength << 1) | segment[segmentNumBits + flagBits + j];
        }

        // Если последний сегмент, отсечь случайные биты
        int dataSize = (i == segments.size() - 1 && usefulBitsLength > 0) ? usefulBitsLength : (maxLenLine - segmentNumBits - usefulBits - crcBits - flagBits);

        // Полезные данные
        std::vector<uint8_t> data(segment.begin(), segment.begin() + segmentNumBits + flagBits + usefulBits + dataSize);

        // Вычисление CRC
        uint64_t computedCRC = computeCRC(data);

        // Полученный CRC из кадра (последние crcBits бит)
        uint64_t receivedCRC = 0;
        for (int j = 0; j < crcBits; ++j) {
            receivedCRC = (receivedCRC << 1) | segment[segmentNumBits + flagBits + usefulBits + dataSize + j];
        }

        // Проверка CRC
        if (computedCRC != receivedCRC) {
            incorrectSegments.push_back(i);
        }
    }

    return incorrectSegments;
}

// Функция скремблирования
// Скремблинг симметричен, поэтому та же функция может быть использована
std::vector<std::vector<uint8_t>> Segmenter::scramble(const std::vector<std::vector<uint8_t>>& data) {

    std::vector<std::vector<uint8_t>> scrambledData = data; // Копируем входные данные
    
    uint64_t lfsr = 0xACE1; // Пример начального значения LFSR (может быть любое ненулевое значение)

    for (auto& row : scrambledData) {
        for (auto& bit : row) {
            uint8_t lfsr_bit = (lfsr >> 0) & 1; // Извлекаем наименее значимый бит (LSB) из текущего значения LFSR
            lfsr = (lfsr >> 1) | (lfsr_bit << 15); // Сдвигаем LFSR вправо и вставляем извлеченный бит на 16-ю позицию
            lfsr ^= (-lfsr_bit) & 0xB400u; // XOR-им LFSR с полиномом, если LSB равен 1 (Таппы: 16 14 13 11)

            bit ^= lfsr_bit; // XOR-им входной бит с сгенерированным псевдослучайным битом
        }
    }

    return scrambledData; // Возвращаем скрамблированные данные
}


void Segmenter::get_size_data_in_slot() const {
    std::cout << "Data bits in slot : " << maxLenLineInSegment << "  |  Input data : " << dataBitsInput << "  |  N_Slots : " << totalSegments <<  std::endl;
}

std::vector<std::vector<uint8_t>> Segmenter::reshape(const std::vector<uint8_t>& bits) {
    std::vector<std::vector<uint8_t>> matrix;
    size_t totalBits = bits.size();
    size_t numRows = (totalBits + maxLenLine - 1) / maxLenLine;  // Округляем вверх

    matrix.reserve(numRows);  // Оптимизация для избежания перераспределения памяти

    for (size_t i = 0; i < totalBits; i += maxLenLine) {
        size_t lineEnd = std::min(i + maxLenLine, totalBits);
        matrix.emplace_back(bits.begin() + i, bits.begin() + lineEnd);
    }

    return matrix;
}

std::vector<uint8_t> Segmenter::extract_data(const std::vector<std::vector<uint8_t>>& bits) {
    auto err_crc = checkCRC(bits);
    if (!err_crc.empty()) {
        std::cout << "Incorrect segments: ";
        for (auto err : err_crc) {
            std::cout << err << " ";
        }
        std::cout << std::endl;
        return {};
    }

    std::vector<uint8_t> data_out;

    for (size_t i = 0; i < bits.size(); ++i) {
        const auto& segment = bits[i];

        // Длина полезных бит (следующие usefulBits бит)
        int usefulBitsLength = 0;
        for (int j = 0; j < usefulBits; ++j) {
            usefulBitsLength = (usefulBitsLength << 1) | segment[segmentNumBits + flagBits + j];
        }

        // Если последний сегмент, отсечь случайные биты
        int dataSize = (i == bits.size() - 1 && usefulBitsLength > 0) ? usefulBitsLength : (maxLenLine - segmentNumBits - usefulBits - crcBits - flagBits);

        // Полезные данные
        std::vector<uint8_t> data(segment.begin() + segmentNumBits + flagBits + usefulBits, segment.begin() + segmentNumBits + flagBits + usefulBits + dataSize);

        data_out.insert(data_out.end(), data.begin(), data.end());
    }
    return data_out;
}

uint8_t Segmenter::extract_flag(const std::vector<std::vector<uint8_t>>& bits) {
    // Предполагается, что флаг находится сразу после номера сегмента
    // и состоит из двух битов
    if (bits.empty() || bits[0].size() < segmentNumBits + 2) {
        throw std::runtime_error("Неверный формат сегментов или недостаточная длина");
    }
    
    uint8_t flag = 0;
    // Извлечение двух битов флага из первого сегмента
    flag |= (bits[0][segmentNumBits] << 1);      // Старший бит флага
    flag |= bits[0][segmentNumBits + 1];          // Младший бит флага

    return flag;
}