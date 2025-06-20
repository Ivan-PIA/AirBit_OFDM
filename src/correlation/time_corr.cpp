
#include "time_corr.hpp"
#include "../../log_level/log.hpp"
#include <algorithm>
#include "../../OFDM/ofdm_mod.h"
#include <omp.h> 
// g++ plot.cpp -o l -lsfml-graphics -lsfml-window -lsfml-syste





// vector<complex<double>> zadoff_chu(int N = 1, int u = 25, bool PSS = false) {
//     if (PSS) {

//         N = 63;
//         vector<complex<double>> ex1(31);

//         for (int n = 0; n < 31; ++n) {
//             ex1[n] = exp(-1i * M_PI * double(u) * double(n) * double(n + 1) / double(N));
//         }

//         vector<complex<double>> ex2(31);

//         for (int n = 31; n < 62; ++n) {
//             ex2[n - 31] = exp(-1i * M_PI * double(u) * double((n + 1)) * double(n + 2) / double(N));
//         }

//         vector<complex<double>> result(62);

//         copy(ex1.begin(), ex1.end(), result.begin());
//         copy(ex2.begin(), ex2.end(), result.begin() + 31);

//         return result;
//     }    

//     else {
//         vector<complex<double>> result(N);

//         for (int n = 0; n < N; ++n) {
//             result[n] = exp(-1i * M_PI * double(u) * double(n) * double((n + 1)) / double(N));
//         }

//         return result;
//     }
// }



vector<complex<double>> pss_on_carrier(int Nfft){

    

    ifstream real_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/OFDM_TX_RX/pss_real.txt");
    if (!real_file.is_open()) {
        std::cerr << "Не удалось открыть файл2" << std::endl;
        
    }
    // файл с мнимой частью
    ifstream imag_file("/home/ivan/Desktop/Work_dir/Yadro/ofdm/OFDM_TX_RX/pss_imag.txt");
    if (!imag_file.is_open()) {
        std::cerr << "Не удалось открыть файл2" << std::endl;
        
    }

    vector<double> real_data;
    vector<double> imag_data;
    vector<complex<double>> pss_ifft;

    double real_num, imag_num;
    while (real_file >> real_num && imag_file >> imag_num) {
        pss_ifft.push_back(complex<double>(real_num, imag_num));
    }
    
    for(complex<double> num:pss_ifft){
        cout << num <<  ' ';
    }

    real_file.close();
    imag_file.close();

    return pss_ifft;
}

void write_to_file(std::vector<double> data){
	FILE *conv;

	conv  = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/conv.txt", "w");
    
	for (int i = 0; i < data.size(); i ++){
		fprintf(conv, "%lf\n", data[i]);
		
	}
	fclose(conv);

}


#define pss_file 0
#define pss_gen  1

int convolve(vector<complex<double>>x) {
    vector<complex<double>> h;
    if (pss_file){
        ifstream real_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_real.txt");
        if (!real_file1.is_open()) {
            std::cerr << "Не удалось открыть файл1" << std::endl;
            return 1;
        }
        // файл с мнимой частью
        ifstream imag_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_imag.txt");
        if (!imag_file1.is_open()) {
            std::cerr << "Не удалось открыть файл1" << std::endl;
            return 1;
        }


        vector<complex<double>> pss_ifft;

        double real_num, imag_num;
        while (real_file1 >> real_num && imag_file1 >> imag_num) {
        pss_ifft.push_back(complex<double>(real_num, imag_num));
        }
        

        real_file1.close();
        imag_file1.close();

        
        for (complex<double> number : pss_ifft) {
            h.push_back(conj(number));
        }
    }

    else{
        OFDM_mod ofdm_mod;

        auto pss = ofdm_mod.mapPSS();
        for (complex<double> number : pss) {
            h.push_back(conj(number));
        }

        reverse(h.begin(), h.end());

    }
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



    // double norm_x;
    // double norm_h;

    // for(int i = 0; i < x.size(); i ++){
    //     norm_x += abs(x[i])* abs(x[i]);
    // }
    // for(int i = 0; i < h.size(); i ++){
    //     norm_h += abs(h[i]) * abs(h[i]);
    // }

    // for(int i = 0; i < x.size(); i ++){
    //     norm_x += norm(x[i]);
    // }
    // for(int i = 0; i < h.size(); i ++){
    //     norm_h += norm(h[i]);
    // }

    complex <double> norm_x;
    complex <double> norm_h;
    for(int i = 0; i < x.size(); i ++){
        norm_x += x[i]* x[i];
    }
    for(int i = 0; i < h.size(); i ++){
        norm_h += h[i] * h[i];
    }

   
    vector <double> abs_y(n);
    
    // int count = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < h.size(); j++) {    
            if (i-j >= 0 && i-j < size_x){
                y[i] += x[i-j] * h[j];
            }
            
        }
        abs_y[i] = abs(y[i]) / (sqrt(abs(norm_h)) * sqrt(abs(norm_x)));
        // abs_y[i] = std::abs(y[i]) / (sqrt(norm_h) * sqrt(norm_x));
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
    
    write_to_file(abs_y);

    int count = 0;
    std::vector <int> index_cor;

    for (int i = 0; i < n; i++){
        //abs_y[i] = abs(y[i]);
        if (abs_y[i] > 1.5){

            // std::cout << "fgkdhbjgdlkgdkfjvdfhlkdhvdhfvh" << std::endl;
            count ++;
            index_cor.push_back(i);
        }
    }
    // count = index_cor[0];
    // std::cout << "index pss" << index_cor[0] << std::endl;
    std::cout << "count = " << count << std::endl;
    return count;
}

vector<complex<double>> convolve2(vector<complex<double>>x, vector<complex<double>>h) {
    
    int n = x.size() + h.size() - 1;
    cout << h.size() << " " << x.size() << endl;
    vector<complex<double>> y(n);

    x.insert(x.begin(), h.size()-1, complex<double>(0, 0));
        
    // cout << y.size() << endl;
    // cout << x.size() << endl;
    int count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < h.size(); j++){       
            y[i] += x[i+j] * h[j];

            
        }
    }

    vector <double> abs_y(n);

    for (int i = 0; i < n; i++){
        //abs_y[i] = sqrt(y[i].real()*y[i].real() + y[i].imag()*y[i].imag());
    }
    

    return y;
}



int detect_pss(const vector<complex<double>>& data, double threshold) {
    
    ifstream real_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_real.txt");
    if (!real_file1.is_open()) {
        std::cerr << "Не удалось открыть файл1" << std::endl;
        return 1;
    }
    // файл с мнимой частью
    ifstream imag_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_imag.txt");
    if (!imag_file1.is_open()) {
        std::cerr << "Не удалось открыть файл1" << std::endl;
        return 1;
    }


    vector<complex<double>> pss_ifft;

    double real_num, imag_num;
    while (real_file1 >> real_num && imag_file1 >> imag_num) {
       pss_ifft.push_back(complex<double>(real_num, imag_num));
    }
    

    real_file1.close();
    imag_file1.close();

    vector<complex<double>> pss;
    for (complex<double> number : pss_ifft) {
        pss.push_back(conj(number));
    }
    
    
    int data_len = data.size();
    int pss_len = pss.size();
    int conv_len = data_len - pss_len + 1;

    // Результат свертки
    vector<complex<double>> conv_r(conv_len);
    vector<double> conv_result(conv_len);
    // Выполнение свертки
    for (int i = 0; i < conv_len; i++) {
        conv_r[i] = 0.0;
        for (int j = 0; j < pss_len; j++) {
            conv_r[i] += data[i + j] * pss[j];
        }
        conv_result[i] = abs(conv_r[i]);
    }

    
    // Нормализация результата свертки
    double min_val = *min_element(conv_result.begin(), conv_result.end());
    double max_val = *max_element(conv_result.begin(), conv_result.end());

    for (double& val : conv_result) {
        val = (val - min_val) / (max_val - min_val);
    }

    // Подсчет количества вхождений PSS
    int pss_count = 0;
    for (double val : conv_result) {
        if (val > 0.9) {
            pss_count++;
        }
    }

    return pss_count;
}

int _conv(const std::vector<std::complex<double>>& y1) {

    OFDM_mod ofdm_mod;
    omp_set_num_threads(16);
    auto y2 = ofdm_mod.mapPSS();

    size_t len1 = y1.size();
    size_t len2 = y2.size();
    size_t maxShift = len1 - len2 + 1;  // Максимальный сдвиг для корреляции
    std::vector<double> result(maxShift, 0.0);

    // Предварительные вычисления нормировочных коэффициентов
    double normY2 = 0.0;
    for (const auto& v : y2) {
        normY2 += std::norm(v);
    }
    normY2 = std::sqrt(normY2);

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

        // Нормирование
        double normFactor = std::sqrt(normY1) * normY2;
        result[shift] = std::abs(sum) / normFactor;
    }

    write_to_file(result);

    int count = 0;
    

    for (int i = 0; i < result.size(); i++){
        //abs_y[i] = abs(y[i]);
        if (result[i] > 0.8){

            // std::cout << "fgkdhbjgdlkgdkfjvdfhlkdhvdhfvh" << std::endl;
            count ++;
            
        }
    }

    return count;
}

// int main(){


//     ifstream real_file("/home/ivan/SDR_libiio_1.x/resurs_file/t_rx_real.txt");
//     if (!real_file.is_open()) {
//         std::cerr << "Не удалось открыть файл3" << std::endl;
//         return 1;
//     }
//     // файл с мнимой частью
//     ifstream imag_file("/home/ivan/SDR_libiio_1.x/resurs_file/t_rx_imag.txt");
//     if (!imag_file.is_open()) {
//         std::cerr << "Не удалось открыть файл3" << std::endl;
//         return 1;
//     }



//     vector <double> real_data;
//     vector <double> imag_data;

//     // istream_iterator<double> it_real(real_file);
//     // istream_iterator<double> it_imag(imag_file);
//     // copy(it_real, istream_iterator<double>(), back_inserter(real_data));
//     // copy(it_imag, istream_iterator<double>(), back_inserter(imag_data));


//     vector<complex<double>> rx_data;

//     double real, imag;
//     while (real_file >> real && imag_file >> imag) {
//         rx_data.push_back(complex<double>(real, imag));
//     }

//     // Закрыть файлы
//     real_file.close();
//     imag_file.close();

//     ifstream real_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_real.txt");
//     if (!real_file1.is_open()) {
//         std::cerr << "Не удалось открыть файл1" << std::endl;
//         return 1;
//     }
//     // файл с мнимой частью
//     ifstream imag_file1("/home/ivan/Desktop/Work_dir/Yadro/ofdm/pss_imag.txt");
//     if (!imag_file1.is_open()) {
//         std::cerr << "Не удалось открыть файл1" << std::endl;
//         return 1;
//     }


//     vector<complex<double>> pss_ifft;

//     double real_num, imag_num;
//     while (real_file1 >> real_num && imag_file1 >> imag_num) {
//        pss_ifft.push_back(complex<double>(real_num, imag_num));
//     }
    
//     for(complex<double> num:rx_data){
//         //cout << num <<  ' ';
//     }

//     real_file1.close();
//     imag_file1.close();

//     vector<complex<double>> conjugate_pss;
//     for (complex<double> number : pss_ifft) {
//         conjugate_pss.push_back(conj(number));
//     }
//     cout << "\n\n\n"<<endl;

// 	FILE *file_conv;

// 	file_conv  = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/conv.txt", "w");
    
//     if(!file_conv){
//         std::cout << "error ept!" << std::endl;
//     }
    
//     vector<double> conv = convolve(rx_data, conjugate_pss);
//     for (double number : conv) {
//         fprintf(file_conv, "%f\n", number);
//     }



//     // vector<complex<double>> conv2 = convolve2(rx_data, conjugate_pss);


    
// }