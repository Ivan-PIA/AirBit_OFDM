#include "soapy_trx.h"

std::atomic<bool> stop_flag(false); // Флаг завершения
std::mutex buffer_mutex;            // Мьютекс для доступа к общему буферу
std::condition_variable buffer_cv;  // Условная переменная для сигнализации
std::vector<std::complex<double>> big_buffer; // Общий большой буфер



// Обработчик сигнала Ctrl+C
void signal_handler(int signum) {
    stop_flag = true; // Установить флаг завершения
    buffer_cv.notify_all(); // Разблокировать условную переменную, если нужно
}



int rx2(const char *output_file) {
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", "usb:");
    SoapySDRKwargs_set(&args, "direct", "1");

    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL) {
        std::cerr << "SoapySDRDevice_make failed: " << SoapySDRDevice_lastError() << std::endl;
        return EXIT_FAILURE;
    }

    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, 3.84e6) != 0) {
        std::cerr << "setSampleRate RX failed: " << SoapySDRDevice_lastError() << std::endl;
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, 1900e6, NULL) != 0) {
        std::cerr << "setFrequency RX failed: " << SoapySDRDevice_lastError() << std::endl;
    }

    size_t channels[] = {0};
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, 1, NULL);
    if (rxStream == NULL) {
        std::cerr << "setupStream RX failed: " << SoapySDRDevice_lastError() << std::endl;
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);

    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);


    int16_t *buffer = (int16_t *)malloc(2 * rx_mtu * sizeof(int16_t));

    FILE *file = fopen(output_file, "w");
    if (file == NULL) {
        std::cerr << "File open failed!" << std::endl;
        return EXIT_FAILURE;
    }

    const long timeoutUs = 400000;
    long long last_time = 0;

    for (size_t i = 0; i < 100; ++i) {
        void *rx_buffs[] = {buffer};
        int flags;
        long long timeNs;

        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        if (sr > 0) {
            fwrite(buffer, 2 * sr * sizeof(int16_t), 1, file);

            if (flags & SOAPY_SDR_HAS_TIME) {
                std::cout << "Time: " << timeNs << ", Delta: " << (timeNs - last_time) << std::endl;
                last_time = timeNs;
            }
        }
    }

    fclose(file);
    free(buffer);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_unmake(sdr);

    return EXIT_SUCCESS;

}

std::vector<std::complex<double>> rx(const char* output_file) {
    // Установка обработчика сигнала
    std::signal(SIGINT, signal_handler);

    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    if (1) {
        SoapySDRKwargs_set(&args, "uri", "usb:");
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.3.1");
    }

    SoapySDRKwargs_set(&args, "direct", "1");
    // SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "loopback", "0");

    SoapySDRDevice* sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL) {
        std::cerr << "SoapySDRDevice_make failed: " << SoapySDRDevice_lastError() << std::endl;
        return {};
    }

    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, FS_2) != 0) {
        std::cerr << "setSampleRate RX failed: " << SoapySDRDevice_lastError() << std::endl;
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, 1900e6, NULL) != 0) {
        std::cerr << "setFrequency RX failed: " << SoapySDRDevice_lastError() << std::endl;
    }

    size_t channels[] = {0};
    SoapySDRStream* rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, 1, NULL);
    if (rxStream == NULL) {
        std::cerr << "setupStream RX failed: " << SoapySDRDevice_lastError() << std::endl;
        SoapySDRDevice_unmake(sdr);
        return {};
    }

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);

    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);

    int16_t* buffer = (int16_t*)malloc(2 * rx_mtu * sizeof(int16_t));
    if (buffer == NULL) {
        std::cerr << "Memory allocation failed!" << std::endl;
        return {};
    }

    FILE* file = fopen(output_file, "w");
    if (file == NULL) {
        std::cerr << "File open failed!" << std::endl;
        free(buffer);
        return {};
    }

    const long timeoutUs = 400000;
    long long last_time = 0;

    while (!stop_flag) { // Цикл работы до сигнала завершения
        std::vector<std::complex<double>> temp_buffer;
        for (size_t i = 0; i < 10 && !stop_flag; ++i) {
            void* rx_buffs[] = { buffer };
            int flags;
            long long timeNs;

            int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
            if (sr > 0) {
                fwrite(buffer, 2 * sr * sizeof(int16_t), 1, file);

                // Конвертируем данные в std::complex<double> и добавляем в temp_buffer
                for (int j = 0; j < sr; ++j) {
                    temp_buffer.emplace_back(buffer[2 * j], buffer[2 * j + 1]);
                }

                if (flags & SOAPY_SDR_HAS_TIME) {
                    std::cout << "Time: " << timeNs << ", Delta: " << (timeNs - last_time) << std::endl;
                    last_time = timeNs;
                }
            }
        }

        // Отправляем буфер на свертку
        if (!temp_buffer.empty() && _conv(temp_buffer) > 1) {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            big_buffer.insert(big_buffer.end(), temp_buffer.begin(), temp_buffer.end());
        }
    }

    fclose(file);
    free(buffer);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_unmake(sdr);

    std::cout << "Завершение программы." << std::endl;
    return big_buffer;
}