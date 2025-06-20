#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <chrono>
#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
// #include "../../OFDM/ofdm_mod.h"
#include "/home/ivan/Desktop/OFDM_transceiver/OFDM/ofdm_mod.h"
#include "../../OFDM/freq_offset.hpp"
#include "../../OFDM/ofdm_demod.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../File_converter/file_converter.h"
#include "../../soapy/soapy_trx.h"
#include "../../soapy/soapy_dev.h"
// #include "../../other/plots.h"
// #include "../../ad9361/ad9361.hpp"

#include "../correlation/time_corr.hpp"

#include <omp.h>

// g++ test2.cpp ../../File_converter/file_converter.cpp ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp -o test && ./test


void remove_cyclic_prefix(
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

double max_magnitude(const std::vector<std::complex<double>>& ofdm_data) {
    auto max_it = std::max_element(ofdm_data.begin(), ofdm_data.end(),
        [](const std::complex<double>& a, const std::complex<double>& b) {
            return std::abs(a) < std::abs(b); // Сравнение по модулю
        });
    
    return std::abs(*max_it); // Вернем максимальный модуль
}

int main() {

    FILE *file = fopen("txdata1.pcm", "w");
    
    // auto bits = generateRandBits(5000,1);
    // auto bits = file2bits("/home/ivan/Desktop/OFDM_transceiver/soapy/soapy_tx.cpp");
    // auto bits = file2bits("/home/ivan/Desktop/OFDM_transceiver/OFDM/freq_offset.cpp");
    // auto bits = file2bits("/home/ivan/Desktop/Work_dir/Yadro/ofdm/OFDM_TX_RX/test/resurs_rx/rx_file.py");
    auto bits = file2bits("/home/ivan/Desktop/OFDM_transceiver/test/test_channel/test_file_in/арбуз_арбуз.jpeg");

    Segmenter segmenter;
    auto segments = segmenter.segment(bits,2); 
    segments = segmenter.scramble(segments);

    QAM_mod qam_mod;
    auto qpsk_mod = qam_mod.modulate(segments, QPSK);

    OFDM_mod ofdm_mod;
    auto ofdm_data = ofdm_mod.modulate(qpsk_mod);
    std::cout << "size_ofdm: " << ofdm_data.size() << std::endl;    

    auto pss = ofdm_mod.mapPSS(0); // PSS во времени для поиска
   
    for (int i = 0; i < (int)ofdm_data.size(); i ++){
        ofdm_data[i] = ofdm_data[i] * 2.0;
        // std::cout << ofdm_data[i] << std::endl;
    }
    std::cout << "Max el:"<<max_magnitude(ofdm_data) << std::endl;
    int rx_mtu = 1920;

    OFDM_demod ofdm_demod;
    // int p = tx_rx(ofdm_data);
    
    SDRHandler sdr;
    auto rx_data = sdr.start_stream_dual(ofdm_data);
    auto start1 = std::chrono::high_resolution_clock::now();
    // auto rx_data = tx_rx(ofdm_data);


    std::cout << "SIZE RX: " << rx_data.size() << std::endl;

    // auto corr_pss = ofdm_demod.correlation(rx_data, pss);


    // auto indexs_pss = ofdm_demod.get_ind_pss(corr_pss, 0.90); 
    // // std::cout << "size ind: " << indexs_pss.size() << std::endl;

    // if(indexs_pss.size() < 1){
    //     std::cout << "PSS not found! " << std::endl;
    //     // return 0;
    // }
    // int ind_first = indexs_pss[0];
    
    // std::cout << "indexs_pss[0] " << indexs_pss[0]<<std::endl;
    // // rx_data.assign(rx_data.begin() + ind_first+N_FFT, rx_data.end());

    // std::vector<std::complex<double>> processed_data;
    // remove_cyclic_prefix(rx_data, processed_data, 32, 128);

    // std::cout << "size processed_data: " << processed_data.size() << std::endl;

    
    // // std::vector<std::complex<double>> data_offset;
    // // frequency_correlation(pss, processed_data, FS_2/N_FFT, data_offset, FS_2);

    // // std::cout << "size data_offset: " << data_offset.size() << std::endl;

    // std::cout << "FS: " << FS_2 << std::endl;
    // std::cout << "N_FFT: " << N_FFT << std::endl;

    // // cool_plot(rx_data, "-", "plot 2 - 2", false);
    
    
    
    // // spectrogram_plot(processed_data);
    
    // // std::vector<cd> rx_data2(ofdm_data.size());
    // // size_t tx_mtu = 1920;

    // // for (int step_mtu = 0; step_mtu < (int)ofdm_data.size() / rx_mtu; step_mtu++) {
    // //     int k = 0;
    // //     int16_t tx_buff[2 * (int)tx_mtu];

    // //     // Инициализация буфера нулями
    // //     memset(tx_buff, 0, sizeof(tx_buff));
    
    // //     for (int i = 0; i < (int)rx_mtu; i++) {
    // //         tx_buff[k] =     static_cast<int16_t>(ofdm_data[((step_mtu) * rx_mtu) + i].real());
    // //         tx_buff[k + 1] = static_cast<int16_t>(ofdm_data[((step_mtu) * rx_mtu) + i].imag());
    // //         k += 2;
    // //     }

    // //     // fwrite(tx_buff, 2 * sizeof(int16_t) * (int)rx_mtu, 1, file);
    // //     int o = 0;
    // //     for (int i = 0; i < rx_mtu * 2; i += 2){
    // //         rx_data2[((step_mtu) * rx_mtu) + o] = std::complex<float>(
    // //             static_cast<float>(tx_buff[i]),
    // //             static_cast<float>(tx_buff[i + 1])
    // //         );
    // //         o++;
    // //     }
    // // }
    // // std::vector<std::complex<double>> processed_data2;
    // // remove_cyclic_prefix(rx_data2, processed_data2, 32, 128);
    // // rx_data2.assign(rx_data2.begin() + ind_first+32, rx_data2.end());
    // // spectrogram_plot(processed_data2);
	// // show_plot();
    // auto demod_signal = ofdm_demod.demodulate(rx_data);
    // std::cout<<"size signal: " << demod_signal.size() << std::endl;
    // // printf("[%s] [%d ] \n", __func__, __LINE__);
    
    // QAM_demod qam_demod;
    // auto demod_bits = qam_demod.demodulate(demod_signal);
	// // printf("[%s] [%d ] \n", __func__, __LINE__);
	
    // auto demod_bits_m = segmenter.reshape(demod_bits);
    // demod_bits_m = segmenter.scramble(demod_bits_m);
    // // printf("[%s] [%d ] \n", __func__, __LINE__);
    // auto data = segmenter.extract_data(demod_bits_m);
    // // printf("[%s] [%d ] \n", __func__, __LINE__);
    // auto flag = segmenter.extract_flag(demod_bits_m);
    // // printf("[%s] [%d ] \n", __func__, __LINE__);

    // // bits2file("/home/ivan/Desktop/OFDM_transceiver/src/test", data);
    // if      (flag == 1) bits2string(data);
    // else if (flag == 2) bits2file("/home/ivan/Desktop/OFDM_transceiver/src/test", data);
    // auto end1 = std::chrono::high_resolution_clock::now();

    // auto dur1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

    // std::cout << "Time:" << dur1.count() << " ms"<< std::endl;

    // std::cout << "Bit rate: " << float(float(bits.size())/float(dur1.count())*1000.0/1000000.0) << " Mbps " << std::endl;
	// // printf("[%s] [%d ] \n", __func__, __LINE__);

    
    return 0;
}
