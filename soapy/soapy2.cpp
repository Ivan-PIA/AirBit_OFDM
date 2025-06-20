#include "soapy2.h"

SDRHandler::SDRHandler(const std::string &ip_addr, bool enable_rx, bool enable_tx) 
    : device(nullptr), rxStream(nullptr), txStream(nullptr),
      rx_mtu(0), tx_mtu(0), sampleRate(1920000), frequency(1900e6),
      rx_gain(30), tx_gain(70), ip(ip_addr) {
    if (initDevice() && configureStreams()) {
        std::cout << "SDR " << ip << " initialized successfully\n";
    }
}

SDRHandler::~SDRHandler() {
    stopStreaming();
    if (device) SoapySDRDevice_unmake(device);
}

bool SDRHandler::initDevice() {
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", ip.c_str());

    device = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (!device) {
        std::cerr << "Device creation failed: " << SoapySDRDevice_lastError() << std::endl;
        return false;
    }

    SoapySDRDevice_setSampleRate(device, SOAPY_SDR_RX, 0, sampleRate);
    SoapySDRDevice_setFrequency(device, SOAPY_SDR_RX, 0, frequency, NULL);
    SoapySDRDevice_setGain(device, SOAPY_SDR_RX, 0, rx_gain);

    SoapySDRDevice_setSampleRate(device, SOAPY_SDR_TX, 0, sampleRate);
    SoapySDRDevice_setFrequency(device, SOAPY_SDR_TX, 0, frequency, NULL);
    SoapySDRDevice_setGain(device, SOAPY_SDR_TX, 0, tx_gain);

    return true;
}

bool SDRHandler::configureStreams() {
    size_t channels[] = {0};
    size_t channel_count = 1;

    rxStream = SoapySDRDevice_setupStream(device, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    txStream = SoapySDRDevice_setupStream(device, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);

    if (!rxStream || !txStream) {
        std::cerr << "Stream setup failed: " << SoapySDRDevice_lastError() << std::endl;
        return false;
    }

    rx_mtu = SoapySDRDevice_getStreamMTU(device, rxStream);
    tx_mtu = SoapySDRDevice_getStreamMTU(device, txStream);

    return true;
}

void SDRHandler::startStreaming() {
    if (rxStream) SoapySDRDevice_activateStream(device, rxStream, 0, 0, 0);
    if (txStream) SoapySDRDevice_activateStream(device, txStream, 0, 0, 0);
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

void runDualSDR(std::vector<std::complex<double>> tx_data) {
    SDRHandler rxSDR("ip:192.168.2.1", true, false);
    SDRHandler txSDR("ip:192.168.3.1", false, true);

    rxSDR.startStreaming();
    txSDR.startStreaming();

    std::cout << "Streaming started!\n";

    std::thread rxThread([&]() {
        std::vector<std::complex<double>> rx_data(10000);
        int16_t buffer[20000];
        void *rxBuffs[] = {buffer};
        int flags;
        long long timeNs;

        while (true) {
            int sr = SoapySDRDevice_readStream(rxSDR.device, rxSDR.rxStream, rxBuffs, 10000, &flags, &timeNs, 100000);
            if (sr > 0) {
                std::cout << "RX received " << sr << " samples\n";
            }
        }
    });

    std::thread txThread([&]() {
        int16_t buffer[20000];
        void *txBuffs[] = {buffer};
        int flags = SOAPY_SDR_HAS_TIME;
        long long timeNs = 0;

        for (size_t i = 0; i < tx_data.size(); i += 10000) {
            SoapySDRDevice_writeStream(txSDR.device, txSDR.txStream, (const void *const *)txBuffs, 10000, &flags, timeNs, 100000);
            timeNs += 10000;
            std::cout << "TX sent 10000 samples\n";
        }
    });

    rxThread.join();
    txThread.join();

    rxSDR.stopStreaming();
    txSDR.stopStreaming();
}
