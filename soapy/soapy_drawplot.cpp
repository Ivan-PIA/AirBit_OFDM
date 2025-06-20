#include "soapy_dev.h"

#include "../src/correlation/time_corr.hpp"

#include "../QAM/qam_mod.h"
#include "../QAM/qam_demod.h"
#include "../Segmenter/segmenter.h"
// #include "../../OFDM/ofdm_mod.h"
#include "/home/ivan/Desktop/OFDM_transceiver/OFDM/ofdm_mod.h"
#include "../OFDM/freq_offset.hpp"
#include "../OFDM/ofdm_demod.h"
#include "../OFDM/ofdm_mod.h"
#include "../File_converter/file_converter.h"
#include "../other/plots.h"

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

SDRHandler::SDRHandler() : device(nullptr), rxStream(nullptr), txStream(nullptr),
                           sampleRate(1.92e6), frequency(2000e6), rx_gain(30), tx_gain(70) {}

SDRHandler::~SDRHandler() {
    stopStreaming();
    if (device) {
        SoapySDRDevice_unmake(device);
    }
}

bool SDRHandler::initDevice(const char *usb, int trx ) {
    printf("[DEBUG] initDevice() called for URI=%s, trx=%d\n", usb, trx);
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", usb);
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "direct", "1");

    device = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);
    
    if (!device) {
        printf("Device creation failed: %s\n", SoapySDRDevice_lastError());
        return false;
    }



    if (trx == 1){
        if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_RX, 0, sampleRate) != 0) {
            printf("Failed to set RX sample rate: %s\n", SoapySDRDevice_lastError());
            
        }
        std::cout<< "SampleRate RX " << SoapySDRDevice_getSampleRate(device, SOAPY_SDR_RX, 0) << std::endl;
        if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_RX, 0, frequency, NULL) != 0) {
            printf("Failed to set RX frequency: %s\n", SoapySDRDevice_lastError());
            
        }
        std::cout<< "Frequency RX " << SoapySDRDevice_getFrequency(device, SOAPY_SDR_RX, 0) << std::endl;
    }
    else if(trx== 2){
        if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, sampleRate) != 0) {
            printf("Failed to set TX sample rate: %s\n", SoapySDRDevice_lastError());
            
        }
        std::cout<< "SampleRate TX " << SoapySDRDevice_getSampleRate(device, SOAPY_SDR_TX, 0) << std::endl;
        if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, frequency, NULL) != 0) {
            printf("Failed to set TX frequency: %s\n", SoapySDRDevice_lastError());
            
        }
        std::cout<< "Frequency TX " << SoapySDRDevice_getFrequency(device, SOAPY_SDR_TX, 0) << std::endl;
    }
    else if(trx== 3){
        if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_RX, 0, sampleRate) != 0) {
            printf("Failed to set RX sample rate: %s\n", SoapySDRDevice_lastError());
            std::cout<< "SampleRate RX "  << SoapySDRDevice_getSampleRate(device, SOAPY_SDR_RX, 0) << std::endl;
        }
        if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_RX, 0, frequency, NULL) != 0) {
            printf("Failed to set RX frequency: %s\n", SoapySDRDevice_lastError());
            std::cout<< "Frequency RX " << SoapySDRDevice_getFrequency(device, SOAPY_SDR_RX, 0) << std::endl;
        }
        if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, sampleRate) != 0) {
            printf("Failed to set TX sample rate: %s\n", SoapySDRDevice_lastError());
            std::cout<< "SampleRate TX " << SoapySDRDevice_getSampleRate(device, SOAPY_SDR_TX, 0) << std::endl;
        }
        if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, frequency, NULL) != 0) {
            printf("Failed to set TX frequency: %s\n", SoapySDRDevice_lastError());
            std::cout<< "Frequency TX " << SoapySDRDevice_getFrequency(device, SOAPY_SDR_TX, 0) << std::endl;
        }
    }

    return true;
}

bool SDRHandler::configureStreams() {
    size_t channels[] = {0};
    size_t channel_count = 1;

    rxStream = SoapySDRDevice_setupStream(device, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (!rxStream) {
        printf("Failed to setup RX stream: %s\n", SoapySDRDevice_lastError());
        return false;
    }
    
    txStream = SoapySDRDevice_setupStream(device, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (!txStream) {
        printf("Failed to setup TX stream: %s\n", SoapySDRDevice_lastError());
        return false;
    }
    

    rx_mtu = SoapySDRDevice_getStreamMTU(device, rxStream);
    tx_mtu = SoapySDRDevice_getStreamMTU(device, txStream);

    std::cout << "RX MTU: " << rx_mtu << std::endl;
    std::cout << "TX MTU: " << tx_mtu << std::endl;

    SoapySDRDevice_setGain(device, SOAPY_SDR_RX, 0, rx_gain);
    SoapySDRDevice_setGain(device, SOAPY_SDR_TX, 0, tx_gain);

    return true;
}

void SDRHandler::startStreaming() {
    SoapySDRDevice_activateStream(device, rxStream, 0, 0, 0);
    SoapySDRDevice_activateStream(device, txStream, 0, 0, 0);
}

void SDRHandler::stopStreaming() {
    if (rxStream) {
        SoapySDRDevice_deactivateStream(device, rxStream, 0, 0);
        SoapySDRDevice_closeStream(device, rxStream);
    }
    if (txStream) {
        SoapySDRDevice_deactivateStream(device, txStream, 0, 0);
        SoapySDRDevice_closeStream(device, txStream);
    }
}

std::queue<std::vector<std::complex<double>>> spectrogramQueue;
std::mutex queueMutex;
std::condition_variable queueCond;
bool stopPlotting = false;


void spectrogramThread() {
    while (true) {
        std::vector<std::complex<double>> data_plot;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCond.wait(lock, []{ return !spectrogramQueue.empty() || stopPlotting; });

            if (stopPlotting && spectrogramQueue.empty()) break;

            data_plot = std::move(spectrogramQueue.front());
            spectrogramQueue.pop();
        }

        // Отрисовка после накопления 10 блоков rx_mtu
        spectrogram_plot_subplot(data_plot, "Spectrogram", 128, true);
    }
}

// void spectrogramThread() {
//     while (true) {
//         std::vector<std::complex<double>> data_plot;
        
//         {
//             std::unique_lock<std::mutex> lock(queueMutex);
//             queueCond.wait(lock, []{ return !spectrogramQueue.empty() || stopPlotting; });

//             if (stopPlotting && spectrogramQueue.empty()) break;

//             data_plot = std::move(spectrogramQueue.front());
//             spectrogramQueue.pop();
//         }

//         // Отрисовка после накопления 10 блоков rx_mtu
//         cool_scatter2(data_plot, "Spectrogram", true);
//     }
// }
bool SDRHandler::initRxDevice(const char *usb) {
    printf("[DEBUG] this=%p, initDevice() URI=%s\n", this, usb);
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", usb);
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "direct", "1");

    device = SoapySDRDevice_make(&args);

    printf("[DEBUG] initRxDevice() URI=%s, device ptr=%p\n", usb, device);
    


    SoapySDRKwargs_clear(&args);

    if (!device) {
        printf("RX Device creation failed: %s\n", SoapySDRDevice_lastError());
        return false;
    }

    if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_RX, 0, sampleRate) != 0) {
        printf("Failed to set RX sample rate: %s\n", SoapySDRDevice_lastError());
    }
    std::cout << "SampleRate RX " << SoapySDRDevice_getSampleRate(device, SOAPY_SDR_RX, 0) << std::endl;

    if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_RX, 0, frequency, NULL) != 0) {
        printf("Failed to set RX frequency: %s\n", SoapySDRDevice_lastError());
    }
    std::cout << "Frequency RX " << SoapySDRDevice_getFrequency(device, SOAPY_SDR_RX, 0) << std::endl;

    return true;
}

bool SDRHandler::initTxDevice(const char *usb) {
    printf("[DEBUG] this=%p, initDevice() URI=%s\n", this, usb);
    
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", usb);
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "direct", "1");

    device = SoapySDRDevice_make(&args);

    printf("[DEBUG] initTxDevice() URI=%s, device ptr=%p\n", usb, device);

    SoapySDRKwargs_clear(&args);

    if (!device) {
        printf("TX Device creation failed: %s\n", SoapySDRDevice_lastError());
        return false;
    }

    if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, sampleRate) != 0) {
        printf("Failed to set TX sample rate: %s\n", SoapySDRDevice_lastError());
    }
    std::cout << "SampleRate TX " << SoapySDRDevice_getSampleRate(device, SOAPY_SDR_TX, 0) << std::endl;

    if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, frequency, NULL) != 0) {
        printf("Failed to set TX frequency: %s\n", SoapySDRDevice_lastError());
    }
    std::cout << "Frequency TX " << SoapySDRDevice_getFrequency(device, SOAPY_SDR_TX, 0) << std::endl;

    return true;
}
bool SDRHandler::configureRxStream() {
    printf("[DEBUG] startStreaming() device ptr=%p\n", device);
    size_t channels[] = {0};
    rxStream = SoapySDRDevice_setupStream(device, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, 1, NULL);
    if (!rxStream) {
        printf("Failed to setup RX stream: %s\n", SoapySDRDevice_lastError());
        return false;
    }

    rx_mtu = SoapySDRDevice_getStreamMTU(device, rxStream);
    std::cout << "RX MTU: " << rx_mtu << std::endl;

    SoapySDRDevice_setGain(device, SOAPY_SDR_RX, 0, rx_gain);
    return true;
}
bool SDRHandler::configureTxStream() {
    size_t channels[] = {0};
    txStream = SoapySDRDevice_setupStream(device, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, 1, NULL);
    if (!txStream) {
        printf("Failed to setup TX stream: %s\n", SoapySDRDevice_lastError());
        return false;
    }

    tx_mtu = SoapySDRDevice_getStreamMTU(device, txStream);
    std::cout << "TX MTU: " << tx_mtu << std::endl;

    SoapySDRDevice_setGain(device, SOAPY_SDR_TX, 0, tx_gain);
    return true;
}

void SDRHandler::startRx() {
    if (rxStream) {
        SoapySDRDevice_activateStream(device, rxStream, 0, 0, 0);
    }
}

void SDRHandler::startTx() {
    if (txStream) {
        SoapySDRDevice_activateStream(device, txStream, 0, 0, 0);
    }
}

void SDRHandler::stopRx() {
    if (rxStream) {
        SoapySDRDevice_deactivateStream(device, rxStream, 0, 0);
        SoapySDRDevice_closeStream(device, rxStream);
        rxStream = nullptr;
    }
}

void SDRHandler::stopTx() {
    if (txStream) {
        SoapySDRDevice_deactivateStream(device, txStream, 0, 0);
        SoapySDRDevice_closeStream(device, txStream);
        txStream = nullptr;
    }
}

std::vector<std::complex<double>> SDRHandler::tx_rx(std::vector<std::complex<double>> tx_data) {
    if (!device || !rxStream || !txStream) {
        printf("SDR device not initialized!\n");
        return {};
    }

    // Запуск потока для отрисовки спектрограммы
    std::thread plotThread(spectrogramThread);

    while (tx_data.size() % (int)rx_mtu != 0) {
        tx_data.push_back(std::complex<double>(0, 0));
    }

    std::vector<std::complex<double>> data(((tx_data.size() / (int)rx_mtu) + 100) * rx_mtu);
    int16_t *buffer = (int16_t *)malloc(2 * (int)rx_mtu * sizeof(int16_t));
    FILE *file = fopen("txdata.pcm", "w");

    std::vector<std::complex<double>> data_plot;  // Буфер для накопления 10 * rx_mtu
    data_plot.reserve(10 * rx_mtu);

    for (size_t step_mtu = 0; step_mtu < ((tx_data.size() / (int)rx_mtu) + 100); step_mtu++) {
        void *rx_buffs[] = {buffer};
        int flags;
        long long timeNs;

        int sr = SoapySDRDevice_readStream(device, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, 10000000);
        if (sr < 0) continue;

        int g = 0;  
        for (int i = 0; i < (int)rx_mtu * 2; i += 2) {
            data[step_mtu * (int)rx_mtu + g] = std::complex<double>(buffer[i], buffer[i + 1]);
            data_plot.push_back(std::complex<double>(buffer[i], buffer[i + 1])); 
            g++;
        }

        // Отправляем в очередь после накопления 10 * rx_mtu
        if (data_plot.size() >= 10 * rx_mtu) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                spectrogramQueue.push(std::move(data_plot));
            }
            queueCond.notify_one();

            // Создаём новый буфер для следующего набора данных
            data_plot.clear();
            data_plot.reserve(10 * rx_mtu);
        }

        if (step_mtu % 10 == 0) {
            std::cout << "RX Step: " << step_mtu << std::endl;
        }

        long long tx_time = timeNs + (4 * sampleRate);

        int16_t tx_buff[2 * (int)tx_mtu];
        void *tx_buffs[] = {tx_buff};
        fwrite(buffer, 2 * sizeof(int16_t) * rx_mtu, 1, file);

        if (step_mtu < tx_data.size() / (int)tx_mtu) {
            int k = 0;
            for (int i = 0; i < (int)tx_mtu; i++) {
                tx_buff[k] = (int16_t)tx_data[step_mtu * (int)tx_mtu + i].real();
                tx_buff[k + 1] = (int16_t)tx_data[step_mtu * (int)tx_mtu + i].imag();
                k += 2;
            }
            flags = SOAPY_SDR_HAS_TIME;
            SoapySDRDevice_writeStream(device, txStream, (const void *const *)tx_buffs, tx_mtu, &flags, tx_time, 10000000);
        }
    }

    // Завершаем поток графиков
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopPlotting = true;
    }
    queueCond.notify_one();
    plotThread.join();

    free(buffer);
    fclose(file);
    
    return data;
}



std::vector<std::complex<double>> SDRHandler::start_stream(std::vector<std::complex<double>> tx_data) {
    SDRHandler sdr;
    char *usb1 = "usb:3.11.5";
    if (!sdr.initDevice(usb1,3) || !sdr.configureStreams()) {
        std::cerr << "Failed to initialize SDR\n";
        return {};
    }

    sdr.startStreaming();
    
    
    std::vector<std::complex<double>> rx_data = sdr.tx_rx(tx_data);
    std::cout<<"Debag" << std::endl;
    sdr.stopStreaming();
    std::cout<<"Debag stopStreaming" << std::endl;
    
    return rx_data;
}

std::vector<std::complex<double>> SDRHandler::tx_rx2(SDRHandler &tx_sdr, SDRHandler &rx_sdr, std::vector<std::complex<double>> tx_data) {
    if (!tx_sdr.device || !tx_sdr.txStream || !rx_sdr.device || !rx_sdr.rxStream) {
        printf("TX or RX SDR not initialized!\n");
        return {};
    }

    std::thread plotThread(spectrogramThread);

    while (tx_data.size() % (int)rx_sdr.rx_mtu != 0) {
        tx_data.push_back(std::complex<double>(0, 0));
    }

    std::vector<std::complex<double>> data(((tx_data.size() / (int)rx_sdr.rx_mtu) + 100) * rx_sdr.rx_mtu);
    int16_t *buffer = (int16_t *)malloc(2 * (int)rx_sdr.rx_mtu * sizeof(int16_t));
    FILE *file = fopen("txdata.pcm", "w");

    std::vector<std::complex<double>> data_plot;
    data_plot.reserve(10 * rx_sdr.rx_mtu);

    for (size_t step_mtu = 0; step_mtu < ((tx_data.size() / (int)rx_sdr.rx_mtu) + 100); step_mtu++) {
        void *rx_buffs[] = {buffer};
        int flags;
        long long timeNs;

        int sr = SoapySDRDevice_readStream(rx_sdr.device, rx_sdr.rxStream, rx_buffs, tx_sdr.rx_mtu, &flags, &timeNs, 10000000);
        if (sr < 0) continue;

        int g = 0;  
        for (int i = 0; i < (int)rx_sdr.rx_mtu * 2; i += 2) {
            data[step_mtu * (int)rx_sdr.rx_mtu + g] = std::complex<double>(buffer[i], buffer[i + 1]);
            data_plot.push_back(std::complex<double>(buffer[i], buffer[i + 1])); 
            g++;
        }

        if (data_plot.size() >= 5 * rx_sdr.rx_mtu) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                spectrogramQueue.push(std::move(data_plot));
            }
            queueCond.notify_one();
            data_plot.clear();
            data_plot.reserve(10 * rx_sdr.rx_mtu);
        }

        if (step_mtu % 10 == 0) {
            std::cout << "RX Step: " << step_mtu << std::endl;
        }

        long long tx_time = timeNs + (4 * tx_sdr.sampleRate);

        int16_t tx_buff[2 * (int)tx_sdr.tx_mtu];
        void *tx_buffs[] = {tx_buff};
        fwrite(buffer, 2 * sizeof(int16_t) * rx_sdr.rx_mtu, 1, file);

        if (step_mtu < tx_data.size() / (int)tx_sdr.tx_mtu) {
            int k = 0;
            for (int i = 0; i < (int)tx_sdr.tx_mtu; i++) {
                tx_buff[k] = (int16_t)tx_data[step_mtu * (int)tx_sdr.tx_mtu + i].real();
                tx_buff[k + 1] = (int16_t)tx_data[step_mtu * (int)tx_sdr.tx_mtu + i].imag();
                k += 2;
            }
            flags = SOAPY_SDR_HAS_TIME;
            SoapySDRDevice_writeStream(tx_sdr.device, tx_sdr.txStream, (const void *const *)tx_buffs, tx_sdr.tx_mtu, &flags, tx_time, 10000000);
        }
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopPlotting = true;
    }
    queueCond.notify_one();
    plotThread.join();

    free(buffer);
    fclose(file);
    
    return data;
}


std::vector<std::complex<double>> SDRHandler::start_stream_dual(std::vector<std::complex<double>> tx_data) {
    SDRHandler tx_sdr;
    SDRHandler rx_sdr;

    const char *tx_usb = "usb:1.5.5";
    const char *rx_usb = "usb:3.25.5";

    // Инициализация TX устройства
    if (!tx_sdr.initTxDevice(tx_usb)) {
        std::cerr << "Failed to initialize TX SDR\n";
        return {};
    }
    if (!tx_sdr.configureTxStream()) {
        std::cerr << "Failed to configure TX stream\n";
        return {};
    }

    // Инициализация RX устройства
    if (!rx_sdr.initRxDevice(rx_usb)) {
        std::cerr << "Failed to initialize RX SDR\n";
        return {};
    }
    if (!rx_sdr.configureRxStream()) {
        std::cerr << "Failed to configure RX stream\n";
        return {};
    }

    // Запуск стримов
    tx_sdr.startTx();
    rx_sdr.startRx();

    // Передача и приём
    std::vector<std::complex<double>> rx_data = tx_rx2(tx_sdr, rx_sdr, tx_data);

    // Остановка стримов
    tx_sdr.stopTx();
    rx_sdr.stopRx();

    return rx_data;
}