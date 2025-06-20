#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <stdint.h>
#include <complex.h>
#include <vector>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../src/correlation/time_corr.hpp"
#include "../config.h"



std::vector<std::complex<double>> tx_rx(std::vector<std::complex<double>> tx_data);

int tx(const std::vector<std::complex<double>>& tx_data);

int rx2(const char *output_file);

std::vector<std::complex<double>> rx(const char *output_file);