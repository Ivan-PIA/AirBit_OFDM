#ifndef SDR_HANDLER_H
#define SDR_HANDLER_H

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <iostream>
#include <vector>
#include <complex>
#include <thread>


class SDRHandler {
public:
    SDRHandler(const std::string &ip_addr, bool enable_rx, bool enable_tx);
    ~SDRHandler();

    bool initDevice();
    bool configureStreams();
    void startStreaming();
    void stopStreaming();

    SoapySDRDevice *device;
    SoapySDRStream *rxStream;
    SoapySDRStream *txStream;
    size_t rx_mtu;
    size_t tx_mtu;
    int sampleRate;
    double frequency;
    int rx_gain;
    int tx_gain;
    std::string ip;
};

void runDualSDR(std::vector<std::complex<double>> tx_data);

#endif // SDR_HANDLER_H
