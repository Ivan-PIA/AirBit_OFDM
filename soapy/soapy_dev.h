#ifndef SDR_HANDLER_H
#define SDR_HANDLER_H

#include <vector>
#include <complex>
#include <thread>
#include <mutex>
#include <future>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <condition_variable>
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Time.h>

struct SDRHandler {
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

    SDRHandler();
    ~SDRHandler();
    
    bool initDevice(const char *usb, int trx);
    bool configureStreams();
    void startStreaming();
    void stopStreaming();
    bool initRxDevice(const char *usb);
    bool initTxDevice(const char *usb);
    bool configureRxStream();
    bool configureTxStream();
    void startTx();
    void startRx();
    void stopRx();
    void stopTx();
    
    std::vector<std::complex<double>> tx_rx(std::vector<std::complex<double>> tx_data);
    std::vector<std::complex<double>> tx_rx2(SDRHandler &tx_sdr, SDRHandler &rx_sdr, std::vector<std::complex<double>> tx_data);
    std::vector<std::complex<double>> start_stream(std::vector<std::complex<double>> tx_data);
    
    // Дополнительные функции
    void sendData(const std::vector<std::complex<double>> &tx_data);
    int receiveData(std::vector<std::complex<double>> &rx_data);
    std::vector<std::complex<double>> rx_process();
    void tx_process(const std::vector<std::complex<double>>& tx_data);
    std::vector<std::complex<double>> startParallelProcessing(const std::vector<std::complex<double>> &tx_data);

    bool configureStreams(bool enable_rx, bool enable_tx);
    std::vector<std::complex<double>> runDualSDR(std::vector<std::complex<double>> &tx_data);
    bool initDevice_duo(bool enable_rx, bool enable_tx, const char *ip);
    std::vector<std::complex<double>> start_stream_dual(std::vector<std::complex<double>> tx_data);
};


#endif // SDR_HANDLER_