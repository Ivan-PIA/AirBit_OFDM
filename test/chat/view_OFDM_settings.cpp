#include <iostream>
#include <limits>
#include "../../config.h"

void viewTransmissionSettings() {
    std::string cp_len = (CP_LEN == 0) ? "Normal" : "Extended";
    std::string modulation = (IQ_MODULATION == 1) ? "BPSK" : (IQ_MODULATION == 2) ? "QPSK" : (IQ_MODULATION == 4) ? "16QAM" : "64QAM";
    double speed = (((N_FFT-G_SUBCAR-N_PILOTS-1) * OFDM_SYM_IN_SLOT) - SEGMENT_NUM_BITS - USEFUL_BITS - CRC_BITS - 2) * IQ_MODULATION;
    speed = speed * 2000; // Умножение на колличество слотов
    speed = speed / 1024; // Перевод в килобиты
    speed = speed / 1024; // Перевод в мегабиты
    std::cout   << "FFT Size  : " << N_FFT << "\n"
                << "CP Length : " << cp_len << "\n"
                << "Modulation: " << modulation << "\n"
                << "Frequency : 2.4 GHz\n"  // Пока заглушка
                << "Cell ID   : " << N_CELL_ID << "\n"
                << "Speed     : " << speed << " Мбит/с\n";

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}