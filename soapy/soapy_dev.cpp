#include "soapy_dev.h"

// #include "../src/correlation/time_corr.hpp"


SDRHandler::SDRHandler() : device(nullptr), rxStream(nullptr), txStream(nullptr),
                           sampleRate(1.92e6), frequency(1900e6), rx_gain(30), tx_gain(70) {}

SDRHandler::~SDRHandler() {
    stopStreaming();
    if (device) {
        SoapySDRDevice_unmake(device);
    }
}

bool SDRHandler::initDevice(const char *usb, int trx) {
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

    if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_RX, 0, sampleRate) != 0) {
        printf("Failed to set RX sample rate: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_RX, 0, frequency, NULL) != 0) {
        printf("Failed to set RX frequency: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, sampleRate) != 0) {
        printf("Failed to set TX sample rate: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, frequency, NULL) != 0) {
        printf("Failed to set TX frequency: %s\n", SoapySDRDevice_lastError());
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

std::vector<std::complex<double>> SDRHandler::tx_rx(std::vector<std::complex<double>> tx_data) {
    if (!device || !rxStream || !txStream) {
        printf("SDR device not initialized!\n");
        return {};
    }
    while ( tx_data.size() % (int)rx_mtu != 0){
        tx_data.push_back(std::complex<double>(0,0));
    }
    std::vector<std::complex<double>> data(((tx_data.size() / (int)rx_mtu) + 100) * rx_mtu);
    
    int16_t *buffer = (int16_t *)malloc(2 * (int)rx_mtu * sizeof(int16_t));


    FILE *file = fopen("txdata.pcm", "w");

    for (size_t step_mtu = 0; step_mtu < ((tx_data.size() / (int)rx_mtu) + 100); step_mtu++) {
        
        void *rx_buffs[] = {buffer};
        int flags;
        long long timeNs;   
        // std::cout<<"Debag" << std::endl;

        int sr = SoapySDRDevice_readStream(device, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, 400000);
        if (sr < 0) continue;

        int g = 0;
        for (int i = 0; i < (int)rx_mtu * 2; i += 2) {
            data[step_mtu * (int)rx_mtu + g] = std::complex<double>(buffer[i], buffer[i + 1]);
            g++;
            // std::cout<<step_mtu * rx_mtu + (k++) << std::endl;
        }   
        // std::cout<<"sample rate: " << sampleRate << std::endl;
        long long tx_time = timeNs + (4 * sampleRate);

        int16_t tx_buff[2 * (int)tx_mtu];
        void *tx_buffs[] = {tx_buff};

        fwrite(buffer, 2 * sizeof(int16_t) * rx_mtu, 1, file);
        int k = 0;
        if (step_mtu < tx_data.size()/(int)tx_mtu) {
            for (int i = 0; i < (int)tx_mtu; i++) {
                tx_buff[k] =   (int16_t)tx_data[((step_mtu) * (int)tx_mtu) + i].real();
                tx_buff[k+1] = (int16_t)tx_data[((step_mtu) * (int)tx_mtu) + i].imag();
                k+=2;
            }
            // std::cout<<"Debag" << std::endl;
            flags = SOAPY_SDR_HAS_TIME;
            // fwrite(tx_buff, 2*sizeof(float)*(int)tx_mtu, 1, file);
            SoapySDRDevice_writeStream(device, txStream, (const void *const *)tx_buffs, tx_mtu, &flags, tx_time, 400000);
            
        }
        
    }

    free(buffer);
    return data;
}


std::vector<std::complex<double>> SDRHandler::start_stream(std::vector<std::complex<double>> tx_data) {
    SDRHandler sdr;
    char *usb1 = "usb:3.11.5";
    if (!sdr.initDevice(usb1,1) || !sdr.configureStreams()) {
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

void SDRHandler::sendData(const std::vector<std::complex<double>> &tx_data) {
    std::cout << "Sending data...\n";
}

int SDRHandler::receiveData(std::vector<std::complex<double>> &rx_data) {
    std::cout << "Receiving data...\n";
    return 0;
}

std::vector<std::complex<double>> rx_process() {
    std::cout << "Processing RX data...\n";
    return {};
}

void SDRHandler::tx_process(const std::vector<std::complex<double>>& tx_data) {
    std::cout << "Processing TX data...\n";
}

std::vector<std::complex<double>> SDRHandler::startParallelProcessing(const std::vector<std::complex<double>> &tx_data) {
    std::cout << "Starting parallel processing...\n";
    return {};
}

bool SDRHandler::configureStreams(bool enable_rx, bool enable_tx) {
    std::cout << "Configuring streams with enable_rx: " << enable_rx << " enable_tx: " << enable_tx << "\n";
    return true;
}

std::vector<std::complex<double>> SDRHandler::runDualSDR(std::vector<std::complex<double>> &tx_data) {
    SDRHandler rxSDR, txSDR;
    if (!rxSDR.initDevice_duo(true, false, "ip:192.168.2.1") || !txSDR.initDevice_duo(false, true,"ip:192.168.3.1")) {
        std::cerr << "Failed to initialize SDRs\n";
        return{};
    }

    rxSDR.startStreaming();
    txSDR.startStreaming();
    
    std::vector<std::complex<double>> rx_data = rxSDR.tx_rx(tx_data);
    
    rxSDR.stopStreaming();
    txSDR.stopStreaming();

    return rx_data;
}

bool SDRHandler::initDevice_duo(bool enable_rx, bool enable_tx, const char *ip) {
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", ip);
    // SoapySDRKwargs_set(&args, "direct", "1");
    // SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    

    device = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (!device) {
        printf("Device creation failed: %s\n", SoapySDRDevice_lastError());
        return false;
    }

    if (enable_rx) {
        SoapySDRDevice_setSampleRate(device, SOAPY_SDR_RX, 0, sampleRate);
        SoapySDRDevice_setFrequency(device, SOAPY_SDR_RX, 0, frequency, NULL);
    }
    if (enable_tx) {
        SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, sampleRate);
        SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, frequency, NULL);
    }

    return true;
}