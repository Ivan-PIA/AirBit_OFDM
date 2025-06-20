// #include "../stream/iiostream-common.hpp" 

#include "../../ad9361/ad9361.hpp"

// #include "../correlation/time_corr.hpp"
#include <vector>
#include <complex> 
#include <iostream> 
#include <chrono>
#include <omp.h> 
#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../OFDM/ofdm_demod.h"
#include "../../OFDM/sequence.h"
#include "../../OFDM/fft/fft.h"
#include "../../File_converter/file_converter.h"
#include "../../OFDM/freq_offset.hpp"
#include "../../other/model_channel.h"
#include "../../other/plots.h"
#include "../process_buffer/processor.h"
// iio_info -a  : show ip

int test_process_buf(std::vector<comp> rx_data){

	OFDM_mod ofdm_mod;
	auto pss = ofdm_mod.mapPSS();
	std::vector<comp> data_offset;
	frequency_correlation(pss, rx_data, 2100000/128 ,data_offset, 2100000);

	// for (int i = 0; i < rx_data.size(); i ++){
	// 	fprintf(file3, "%d\n", (int16_t)rx_data[i].imag());
	// 	fprintf(file4, "%d\n", (int16_t)rx_data[i].real());
	// }
	// fclose(file3);
	// fclose(file4);


	// cool_plot(data_offset, "-", "plot", false);
	// resource_grid(data_offset, 10,6, 128, 32, "rx", false);
	// cool_scatter(data_offset, "plot", false);
	// show_plot();

	OFDM_demod ofdm_demod;
    auto demod_signal = ofdm_demod.demodulate(data_offset);

	cool_scatter(demod_signal, "qam", false);
	show_plot();
    QAM_demod qam_demod;
    auto demod_bits = qam_demod.demodulate(demod_signal);
	
	Segmenter segmenter;
    auto demod_bits_m = segmenter.reshape(demod_bits);
    demod_bits_m = segmenter.scramble(demod_bits_m);

    auto data = segmenter.extract_data(demod_bits_m);
    auto flag = segmenter.extract_flag(demod_bits_m);

    if      (flag == 1) bits2string(data);
    else if (flag == 2) bits2file("/home/ivan/Desktop/OFDM_transceiver/src/test", data);
	printf("[%s] [%d ] \n", __func__, __LINE__);

	return 1;
}

std::vector<double> _conv(const std::vector<std::complex<double>>& y1, int u) {

    OFDM_mod ofdm_mod;

    auto y2 = ofdm_mod.mapPSS(u);

    size_t len1 = y1.size();
    size_t len2 = y2.size();
    // std::cout << len1 << " " << len2 << std::endl;
    size_t maxShift = len1 - len2 + 1;  // Максимальный сдвиг для корреляции
    std::vector<double> result(maxShift, 0.0);

    // Предварительные вычисления нормировочных коэффициентов
    double normY2 = 0.0;
    
    for (const auto& v : y2) {
        normY2 += std::norm(v);
    }
    normY2 = std::sqrt(normY2);
    // std::vector<double> norm_cor(maxShift);
    // Цикл по сдвигам
    #pragma omp parallel for
    for (size_t shift = 0; shift < maxShift; ++shift) {
        std::complex<double> sum(0.0, 0.0);
        double normY1 = 0.0;

        // Вычисление корреляции на текущем сдвиге
        
        for (size_t i = 0; i < len2; ++i) {
            sum += y1[i + shift] * std::conj(y2[i]);
            normY1 += std::norm(y1[i + shift]);
        }
        // norm_cor.push_back(std::abs(sum));
        // norm_cor[shift] = 
        // Нормирование
        double normFactor = std::sqrt(normY1) * normY2;
        result[shift] = std::abs(sum) / normFactor;
    }

    // write_to_file(result);

    int count = 0;
    

    // for (int i = 0; i < result.size(); i++){
    //     //abs_y[i] = abs(y[i]);
    //     if (result[i] > 0.9){

    //         // std::cout << "fgkdhbjgdlkgdkfjvdfhlkdhvdhfvh" << std::endl;
    //         count ++;
            
    //     }
    // }

    return result;
}


std::vector <double> convolve(vector<complex<double>>x, int u) {
    vector<complex<double>> h;

	OFDM_mod ofdm_mod;

	auto pss = ofdm_mod.mapPSS(u);
	for (complex<double> number : pss) {
		h.push_back(conj(number));
	}

	reverse(h.begin(), h.end());


     // flip !!!!!!!!

    int size_x = x.size();

    int n = x.size() + h.size() - 1;
    
    // show_log(CONSOLE, "[ size = %d ]\n", x.size());
    // cout << h.sizee() << " " << x.size() << endl;
    vector<complex<double>> y(n);

    vector<complex<double>> temp = x;

    temp.insert(temp.begin(), h.size()-1, complex<double>(0, 0));
        
    // cout << y.size() << endl;
    // cout << x.size() << endl;



    double norm_x;
    double norm_h;

    for(int i = 0; i < x.size(); i ++){
        norm_x += abs(x[i])* abs(x[i]);
    }
    for(int i = 0; i < h.size(); i ++){
        norm_h += abs(h[i]) * abs(h[i]);
    }

    // for(int i = 0; i < x.size(); i ++){
    //     norm_x += norm(x[i]);
    // }
    // for(int i = 0; i < h.size(); i ++){
    //     norm_h += norm(h[i]);
    // }

    // complex <double> norm_x;
    // complex <double> norm_h;
    // for(int i = 0; i < x.size(); i ++){
    //     norm_x += x[i]* x[i];
    // }
    // for(int i = 0; i < h.size(); i ++){
    //     norm_h += h[i] * h[i];
    // }

   
    vector <double> abs_y(n);
    
    // int count = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < h.size(); j++) {    
            if (i-j >= 0 && i-j < size_x){
                y[i] += x[i-j] * h[j];
            }
            
        }
        // abs_y[i] = abs(y[i]) / (sqrt(abs(norm_h)) * sqrt(abs(norm_x)));
        abs_y[i] = std::abs(y[i]) / (sqrt(norm_h) * sqrt(norm_x));
        // cout << abs_y[i] << " "<< endl;
        
        // if (abs_y[i] > 0.5){
        //     std::cout << "fgkdhbjgdlkgdkfjvdfhlkdhvdhfvh" << std::endl;
        //     count++;
        // }

    }
    
    // for (size_t i = 0; i < abs_y.size(); ++i) {
    //     for (size_t j = 0; j < h.size(); ++j) {
    //         if (i >= j && i - j < x.size()) {
    //             y[i] += x[i - j] * h[j];
    //         }
    //     }

    //     abs_y[i] = std::abs(y[i]) / (std::sqrt(norm_h) * std::sqrt(norm_x));

    //     if (abs_y[i] > 0.02) {
    //         count++;
    //     }
    // }
    
    // write_to_file(abs_y);

    int count = 0;
    // std::vector <int> index_cor;

    // for (int i = 0; i < n; i++){
    //     //abs_y[i] = abs(y[i]);
    //     if (abs_y[i] > 1.5){

    //         // std::cout << "fgkdhbjgdlkgdkfjvdfhlkdhvdhfvh" << std::endl;
    //         count ++;
    //         index_cor.push_back(i);
    //     }
    // }
    // count = index_cor[0];
    // std::cout << "index pss" << index_cor[0] << std::endl;
    std::cout << "count = " << count << std::endl;
    return abs_y;
}


std::vector<cd> convolve_fft(const std::vector<cd>& vec1, const std::vector<cd>& vec2) {
    int n1 = vec1.size(); 
    int n2 = vec2.size(); 
    int n = nearest_power_of_two(n1 + n2 - 1); // Определяем размер результата корреляции как ближайшую степень двойки

    std::vector<cd> padded_vec1(n, 0);
    std::vector<cd> padded_vec2(n, 0);

    // Копируем данные из vec1 в padded_vec1
    std::copy(vec1.begin(), vec1.end(), padded_vec1.begin());
    std::copy(vec2.rbegin(), vec2.rend(), padded_vec2.begin());

    // Берем комплексно-сопряженное значение второго вектора для корреляции
    // #pragma omp parallel for
    for (int i = 0; i < n2; ++i) {
        padded_vec2[i] = std::conj(padded_vec2[i]);
    }

    // Выполняем FFT для обоих векторов
    auto fft_vec1 = fft(padded_vec1);
    auto fft_vec2 = fft(padded_vec2);

    // Элемент-wise умножение в частотной области
    std::vector<cd> fft_product(n);
    
    for (int i = 0; i < n; ++i) {
        fft_product[i] = fft_vec1[i] * fft_vec2[i]; // Умножаем элементы FFT двух векторов
    }

    // Выполняем IFFT, чтобы получить результат корреляции
    std::vector<cd> result = ifft(fft_product);

    return result; // Возвращаем результат корреляции
}

int main (int argc, char **argv)
{
	char *ip = argv[1];

	FILE *file3, *file4,*file1, *file2;

	file3  = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/rx_imag_file.txt", "w");
    file4 = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/rx_real_file.txt", "w");

	file1  = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/freq_imag_file.txt", "w");
    file2 = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/freq_real_file.txt", "w");

	signal(SIGINT, handle_sig);

	if (!file3){
		std::cout << "error ept!" << std::endl;
	}

	std::vector<comp> rx_data;

    

	// rx_data = read_from_block_real(ip);
	// rx_data = read_from_block_real(ip);
	rx_data = read_from_block(ip, 10);
	std::cout << "size rx data: " << rx_data.size() << std::endl;
	
	for (int i = 0; i < rx_data.size(); i ++){
		fprintf(file3, "%d\n", (int16_t)rx_data[i].imag());
		fprintf(file4, "%d\n", (int16_t)rx_data[i].real());
	}
	fclose(file3);
	fclose(file4);
    OFDM_mod ofdm_mod;

    auto y2 = ofdm_mod.mapPSS(2);
    auto y0 = ofdm_mod.mapPSS(0);
    auto y1 = ofdm_mod.mapPSS(1);
    
	// std::vector<double> conv1 = convolve(rx_data, 0);
	// std::vector<double> conv2 = convolve(rx_data, 1);
	// std::vector<double> conv3 = convolve(rx_data, 2);
	// std::vector<double> conv1 = _conv(rx_data, 0);
	// std::vector<double> conv2 = _conv(rx_data, 1);
	// std::vector<double> conv3 = _conv(rx_data, 2);
    // std::vector<double> conv_pss0 = convolve(y2, 0);
    // std::vector<double> conv_pss1 = convolve(y2, 1);
    std::vector<cd> conv_pss2 = convolve_fft(y2, y2);

    std::cout << "\nBuffer size: " << BLOCK_SIZE*10 << std::endl;

    auto start1 = std::chrono::high_resolution_clock::now();
    std::vector<cd> conv_fft1 = convolve_fft(rx_data, y1);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto dur1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);

    std::cout << "Time:" << std::endl;
    std::cout << "Convolve FFT: " << dur1.count() << " ms" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    std::vector<double> conv2 = _conv(rx_data, 1);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto dur2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);

    std::cout << "Convolve Base: " << dur2.count() << " ms\n" <<std::endl;

	// resource_grid(rx_data, 10,6, 128, 32, "rx", false);
	// cool_scatter(rx_data, "plot", false);
	// // process_buffers(rx_data);
	// cool_plot(conv1, "-", "plot 0 ", false);
	// cool_plot(conv2, "-", "plot 1", false);
	// cool_plot(conv3, "-", "plot 2", false);
	// cool_plot(conv_pss0, "-", "plot 2 - 0 ", false);
	// cool_plot(conv_pss1, "-", "plot 2 - 1", false);
	// cool_plot(conv_pss2, "-", "plot 2 - 2", false);
    cool_plot(conv_fft1, "-", "plot 2 - 2", false);
    cool_plot(conv_pss2, "-", "plot 2 - 2", false);
	// show_plot();

    // spectrogram_plot(rx_data);
	auto pss1 = ofdm_mod.mapPSS(1);

	std::vector<comp> data_offset;

    std::cout << "FS: " << FS << std::endl;
    std::cout << "N_FFT: " << N_FFT << std::endl;

	frequency_correlation(pss1, rx_data, FS/N_FFT ,data_offset, FS);

    // for (int i = 0; i < data_offset.size(); i ++){
	// 	fprintf(file1, "%d\n", (int16_t)data_offset[i].imag());
	// 	fprintf(file2, "%d\n", (int16_t)data_offset[i].real());
	// }

    spectrogram_plot(rx_data);
    show_plot();
    
    // fclose(file1);
	// fclose(file2);
	// test_process_buf(rx_data);
	// std::vector<comp> rx_data;

	// int conv = convolve(rx_data);
	// std::cout << "conv: " << conv << std::endl;
	// int conv = detect_pss(rx_data, 0.9);
	// std::cout << "conv: " << conv << std::endl;
	shutdown();

	std::cout << "Receve done!" << std::endl;

	return 0;
}