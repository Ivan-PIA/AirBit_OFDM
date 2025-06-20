#include "soapy_trx.h"



std::vector<std::vector<std::complex<double>>> split_vector_1920_buff(const std::vector<std::complex<double>>& tx_data) {
    std::vector<std::vector<std::complex<double>>> result;

    // Размер блока
    const size_t block_size = 1920;

    // Количество блоков
    size_t total_blocks = (tx_data.size() + block_size - 1) / block_size;

    // Разделяем вектор на блоки
    for (size_t i = 0; i < total_blocks; ++i) {
        size_t start = i * block_size;
        size_t end = std::min(start + block_size, tx_data.size());
        
        // Создаем блок и копируем данные
        std::vector<std::complex<double>> block(tx_data.begin() + start, tx_data.begin() + end);
        
        // Дополняем нулями, если блок меньше 1920 элементов
        if (block.size() < block_size) {
            std::cout<< "Zeros count1" << std::endl;
            block.resize(block_size, std::complex<double>(0.0, 0.0));
        }
        
        result.push_back(std::move(block));
    }

    return result;
}

std::vector<std::complex<double>> tx_rx(std::vector<std::complex<double>>  tx_data ){
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    if (1) {
        SoapySDRKwargs_set(&args, "uri", "usb:");
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.3.1");
    }

    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "loopback", "0");
    
    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL)
    {
        printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
        return {};
    }

    // Настраиваем устройства RX\TX
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, FS_2) != 0)
    {
        printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, 1900e6, NULL) != 0)
    {
        printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, FS_2) != 0)
    {
        printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, 1900e6, NULL) != 0)
    {
        printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
    }
    printf("SoapySDRDevice_getFrequency tx: %lf\n", SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0));
    
    // Настройка каналов и стримов
    size_t channels[] = {0}; // {0} or {0, 1} 
    // TODO: - с вариантом {0, 1} необходимо разробраться, не работают 2 канала

    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (rxStream == NULL)
    {
        printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return {};
    }
    if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 30.0) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }
    if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], 70.0) !=0 ){
        printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());
    }
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    if (txStream == NULL)
    {
        printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
        SoapySDRDevice_unmake(sdr);
        return {};
    }

    // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    printf("MTU - TX: %lu, RX: %lu\n", tx_mtu, rx_mtu);
    
    printf("Rx gain:%f - Tx gain:%f\n", SoapySDRDevice_getGain(sdr, SOAPY_SDR_RX, channels[0]), SoapySDRDevice_getGain(sdr, SOAPY_SDR_TX, channels[0]));


    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming


    printf("Start test...\n");
    const long  timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)
    int16_t *buffer = (int16_t *)malloc(2 * rx_mtu * sizeof(int16_t));

    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return {};
    }
    // При первом запуске отчищаем rx_buff от возможного мусора.
    for (size_t buffers_read = 0; buffers_read < 128;  buffers_read++)
    {
        void *buffs[] = {buffer}; //void *buffs[] = {rx_buff[0][0], rx_buff[0][1]}; //array of buffers
        int flags; //flags set by receive operation
        long long timeNs; //timestamp for receive buffer
        // Read samples
        int sr = SoapySDRDevice_readStream(sdr, rxStream, buffs, rx_mtu, &flags, &timeNs, timeoutUs); // 100ms timeout
        if (sr < 0)
        {
            // Skip read on error (likely timeout)
            continue;
        }
        // Increment number of buffers read
       
    }

    long long last_time = 0;


    // Создаем файл для записи сэмплов с rx_buff
    FILE *file = fopen("txdata.pcm", "w");

    printf("Start test...\n");
    // std::vector<std::vector<std::complex<double>>> tx_split = split_vector_1920_buff(tx_data);
    while ( tx_data.size() % rx_mtu != 0){
        tx_data.push_back(std::complex<double>(0,0));
    }
    
    std::vector<std::complex<double>>data;
    // Начинается работа с получением и отправкой сэмплов

    // std::cout << "tx_split1: "<< tx_split.size() << std::endl;

    int num_buff = 0;
    printf("tx len: %d\n",tx_data.size());
    printf("rx:%d - tx:%d\n", rx_mtu, tx_mtu);
    printf("chek / :%d\n", tx_data.size()/rx_mtu);

    // std::complex<double> *tx_fast[tx_data.size()];

    std::vector<std::complex<double>> data2(((tx_data.size()/rx_mtu)+100)*rx_mtu);


    for (size_t step_mtu = 0; step_mtu < ((tx_data.size()/rx_mtu)+100); step_mtu++)
    {
        // printf("step buff : %d\n", step_mtu);
        void *rx_buffs[] = {buffer};
        int flags;        // flags set by receive operation
        long long timeNs; //timestamp for receive buffer

        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        if (sr < 0)
        {
            // Skip read on error (likely timeout)
            continue;
        }
        
        int16_t tx_2[2*tx_mtu];

        if (step_mtu <= tx_data.size()/rx_mtu-1){
            int k2 = 0;
            for (int i = 0; i < (int)rx_mtu; i++)
            {
                tx_2[k2]   = (int16_t)tx_data[((step_mtu) * rx_mtu) + i].real();
                tx_2[k2+1] = (int16_t)tx_data[((step_mtu) * rx_mtu) + i].imag();
                k2 += 2;
            }
        }
        fwrite(buffer, 2 * rx_mtu * sizeof(int16_t), 1, file);   
    
        int k = 0;
        if (step_mtu <= ((tx_data.size()/rx_mtu)+100)-1){
            for(int i = 0; i < rx_mtu * 2; i += 2){

                // std::complex<double> z((int16_t)buffer[i],(int16_t)buffer[i+1]);
                // data.push_back(z);
                data2[((step_mtu) * rx_mtu) + k] = std::complex<double>(static_cast<int16_t>(buffer[i]),static_cast<int16_t>(buffer[i+1]));  // Добавлены скобки
                
                k++; //1920
            }
        }
        long long tx_time = timeNs + (4 * FS_2);

        int16_t tx_buff[2*tx_mtu];

        void *tx_buffs[] = {tx_buff};
        if( (step_mtu >= 0) ){

            int k = 0;

            
                //num_buff = 0;
                if (step_mtu <= tx_data.size()/rx_mtu-1){

                    for (int i = 0; i < (int)rx_mtu; i++)
                    {
                        tx_buff[k]   = (int16_t)tx_data[((step_mtu ) * rx_mtu) + i].real();
                        tx_buff[k+1] = (int16_t)tx_data[((step_mtu ) * rx_mtu) + i].imag();
                        k+=2;
                    }
                    num_buff++;

                    flags = SOAPY_SDR_HAS_TIME;
                    int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, 400000);
                    if ((size_t)st != tx_mtu)
                        {
                            printf("TX Failed: %i\n", st);
                        }

                }
     
        }
        
    }
    fclose(file);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);

    SoapySDRDevice_unmake(sdr);

    // Process each rx buffer, looking for transmitted timestamp
    for (size_t j = 0; j < channel_count; j++)
    {
        printf("Checking channel %zu\n", j);

    }


    printf("test complete!\n");


    return data2;
}

// std::vector<std::complex<double>> tx_rx(std::vector<std::complex<double>>  tx_data ){
//     SoapySDRKwargs args = {};
//     SoapySDRKwargs_set(&args, "driver", "plutosdr");
//     if (1) {
//         SoapySDRKwargs_set(&args, "uri", "usb:");
//     } else {
//         SoapySDRKwargs_set(&args, "uri", "ip:192.168.3.1");
//     }

//     SoapySDRKwargs_set(&args, "direct", "1");
//     SoapySDRKwargs_set(&args, "timestamp_every", "1920");
//     SoapySDRKwargs_set(&args, "loopback", "0");
    
//     SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
//     SoapySDRKwargs_clear(&args);

//     if (sdr == NULL)
//     {
//         printf("SoapySDRDevice_make fail: %s\n", SoapySDRDevice_lastError());
//         return {};
//     }

//     // Настраиваем устройства RX\TX
//     if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, FS_2) != 0)
//     {
//         printf("setSampleRate rx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, 1900e6, NULL) != 0)
//     {
//         printf("setFrequency rx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, FS_2) != 0)
//     {
//         printf("setSampleRate tx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, 1900e6, NULL) != 0)
//     {
//         printf("setFrequency tx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     printf("SoapySDRDevice_getFrequency tx: %lf\n", SoapySDRDevice_getFrequency(sdr, SOAPY_SDR_TX, 0));
    
//     // Настройка каналов и стримов
//     size_t channels[] = {0}; // {0} or {0, 1} 
//     // TODO: - с вариантом {0, 1} необходимо разробраться, не работают 2 канала

//     size_t channel_count = sizeof(channels) / sizeof(channels[0]);
//     SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
//     if (rxStream == NULL)
//     {
//         printf("setupStream rx fail: %s\n", SoapySDRDevice_lastError());
//         SoapySDRDevice_unmake(sdr);
//         return {};
//     }
//     if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 10.0) !=0 ){
//         printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -60.0) !=0 ){
//         printf("setGain tx fail: %s\n", SoapySDRDevice_lastError());
//     }
//     SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
//     if (txStream == NULL)
//     {
//         printf("setupStream tx fail: %s\n", SoapySDRDevice_lastError());
//         SoapySDRDevice_unmake(sdr);
//         return {};
//     }

//     // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
//     size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
//     size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
//     printf("MTU - TX: %lu, RX: %lu\n", tx_mtu, rx_mtu);
    
//     printf("Rx gain:%d - Tx gain:%d\n", SoapySDRDevice_getGain(sdr, SOAPY_SDR_RX, channels[0]), SoapySDRDevice_getGain(sdr, SOAPY_SDR_TX, channels[0]));


//     //prepare fixed bytes in transmit buffer
//     //we transmit a pattern of FFFF FFFF [TS_0]00 [TS_1]00 [TS_2]00 [TS_3]00 [TS_4]00 [TS_5]00 [TS_6]00 [TS_7]00 FFFF FFFF
//     //that is a flag (FFFF FFFF) followed by the 64 bit timestamp, split into 8 bytes and packed into the lsb of each of the DAC words.
//     //DAC samples are left aligned 12-bits, so each byte is left shifted into place
//     // for(size_t i = 0; i < 2; i++)
//     // {
//     //     tx_buff[0 + i] = 0xffff;
//     //     // 8 x timestamp words
//     //     tx_buff[10 + i] = 0xffff;
//     // }

//     //activate streams
//     SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
//     SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming

//     //here goes
//     printf("Start test...\n");
//     const long  timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)
//     int16_t *buffer = (int16_t *)malloc(2 * rx_mtu * sizeof(int16_t));

//     if (buffer == NULL) {
//         printf("Memory allocation failed\n");
//         return {};
//     }
//     // При первом запуске отчищаем rx_buff от возможного мусора.
//     for (size_t buffers_read = 0; buffers_read < 128; /* in loop */)
//     {
//         void *buffs[] = {buffer}; //void *buffs[] = {rx_buff[0][0], rx_buff[0][1]}; //array of buffers
//         int flags; //flags set by receive operation
//         long long timeNs; //timestamp for receive buffer
//         // Read samples
//         int sr = SoapySDRDevice_readStream(sdr, rxStream, buffs, rx_mtu, &flags, &timeNs, timeoutUs); // 100ms timeout
//         if (sr < 0)
//         {
//             // Skip read on error (likely timeout)
//             continue;
//         }
//         // Increment number of buffers read
//         buffers_read++;
//     }

//     long long last_time = 0;


//     // Создаем файл для записи сэмплов с rx_buff
//     FILE *file = fopen("txdata.pcm", "w");
    
//     // Количество итерация
//     size_t iteration_count = 30;

//     std::vector<std::vector<std::complex<double>>> tx_split = split_vector_1920_buff(tx_data);
//     std::vector<std::complex<double>>data;
//     // Начинается работа с получением и отправкой сэмплов

//     std::cout << "tx_split1: "<< tx_split.size() << std::endl;

//     int num_buff = 0;

//     for (size_t buffers_read = 0; buffers_read < (tx_split.size()+100); buffers_read++)
//     {
        
//         void *rx_buffs[] = {buffer};
//         int flags;        // flags set by receive operation
//         long long timeNs; //timestamp for receive buffer

//         int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
//         if (sr < 0)
//         {
//             // Skip read on error (likely timeout)
//             continue;
//         }
//         // printf("timeNs - %lli \n", timeNs);
//         // printf("last-time - %lli \n", last_time);
//         // Dump info
//         // printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
//         int num_buff1 = 0;
//         int16_t tx_2[2*tx_mtu];
//             int k2 = 0;
//             for (int i = 0; i < (int)rx_mtu; i++)
//             {
//                 tx_2[k2] = (int16_t)tx_split[num_buff1][i].real();
//                 tx_2[k2+1] = (int16_t)tx_split[num_buff1][i].imag();
//                 k2+=2;
//             }
//             fwrite(tx_2, 2 * rx_mtu * sizeof(int16_t), 1, file);

//             num_buff1++;        
        

//         int k = 0;
//         for(int i = 0; i < rx_mtu*2; i+=2){
//             std::complex<double> z((int16_t)buffer[i],(int16_t)buffer[i+1]);
//             data.push_back(z);
            
//         }
             
//         // last_time = timeNs;

//         // Calculate transmit time 4ms in future
//         long long tx_time = timeNs + (4 * FS_2);

//         // // Set samples
//         // for(size_t i = 0; i < 8; i++)
//         // {
//         //     // Extract byte from tx time
//         //     uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;

//         //     // Add byte to buffer
//         //     tx_buff[2 + i] = tx_time_byte << 4;
//         // }
//         int16_t tx_buff[2*tx_mtu];

//         //заполнение tx_buff значениями сэмплов первые 16 бит - I, вторые 16 бит - Q.

//         // void *tx_buffs[] = {tx_buff};
//         // flags = SOAPY_SDR_HAS_TIME;
//         // int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, 400000);
//         // if ((size_t)st != tx_mtu){
//         //     printf("TX Failed: %i\n", st);
//         // }
//         // Send buffer
//         void *tx_buffs[] = {tx_buff};
//         if( (buffers_read >= 0) ){

//             int k = 0;

//             if (num_buff < (int)tx_split.size()){
//                 //num_buff = 0;
            

//                 for (int i = 0; i < (int)rx_mtu; i++)
//                 {
//                     tx_buff[k] = (int16_t)tx_split[num_buff][i].real();
//                     tx_buff[k+1] = (int16_t)tx_split[num_buff][i].imag();
//                     k+=2;
//                 }
//                 num_buff++;

//                 flags = SOAPY_SDR_HAS_TIME;
//                 int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, 400000);
//                 if ((size_t)st != tx_mtu)
//                     {
//                         printf("TX Failed: %i\n", st);
//                     }
//             }
//         }
        
//     }
//     fclose(file);

//     //stop streaming
//     SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
//     SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

//     //shutdown the stream
//     SoapySDRDevice_closeStream(sdr, rxStream);
//     SoapySDRDevice_closeStream(sdr, txStream);

//     //cleanup device handle
//     SoapySDRDevice_unmake(sdr);

//     // Process each rx buffer, looking for transmitted timestamp
//     for (size_t j = 0; j < channel_count; j++)
//     {
//         printf("Checking channel %zu\n", j);
//         //check_channel(sample_count, channel_count, j, tx_timestamps, rx_timestamps, (uint16_t**)rx_buff, rx_mtu);
//     }

//     //all done
//     printf("test complete!\n");

//     // //free buffers
//     // for (size_t i = 0; i < sample_count; i++)
//     // for (size_t j = 0; j < channel_count; j++)
//     // {
//     //     //free(rx_buff);
//     //     //rx_buff[i][j] = NULL;
//     // }

//     return data;
// }




int tx(const std::vector<std::complex<double>>& tx_data) {
    
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

    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);

    if (sdr == NULL) {
        std::cerr << "SoapySDRDevice_make failed: " << SoapySDRDevice_lastError() << std::endl;
        return EXIT_FAILURE;
    }

    // Настройка передатчика
    if (SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, FS_2) != 0) {
        std::cerr << "setSampleRate TX failed: " << SoapySDRDevice_lastError() << std::endl;
    }
    if (SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, 1900e6, NULL) != 0) {
        std::cerr << "setFrequency TX failed: " << SoapySDRDevice_lastError() << std::endl;
    }

    size_t channels[] = {0};
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, 1, NULL);
    if (txStream == NULL) {
        std::cerr << "setupStream TX failed: " << SoapySDRDevice_lastError() << std::endl;
        SoapySDRDevice_unmake(sdr);
        return EXIT_FAILURE;
    }

    if(SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 10.0) !=0 ){
        printf("setGain rx fail: %s\n", SoapySDRDevice_lastError());
    }
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);

    const size_t buffer_size = 1920; // Фиксированный размер буфера
    int16_t buffer[2 * buffer_size]; // Каждый семпл состоит из I и Q (2 значения)

    // Разделение данных на блоки   
    size_t total_blocks = (tx_data.size() + buffer_size - 1) / buffer_size;

    for (size_t block = 0; block < total_blocks; ++block) {
        size_t start = block * buffer_size;
        size_t end = std::min(start + buffer_size, tx_data.size());

        // Заполнение буфера данными
        size_t idx = 0;
        for (size_t i = start; i < end; ++i) {
            buffer[idx] = (int16_t)tx_data[i].real(); // I
            buffer[idx+1] = (int16_t)tx_data[i].imag(); // Q
            idx+=2;
        }   

        // Дополнение буфера нулями, если блок меньше 1920
        for (size_t i = idx; i < 2 * buffer_size; ++i) {
            buffer[i] = 0;
        }   

        
        // Установка времени передачи
        long long tx_time = 4 * FS_2; // Временная метка в наносекундах
        int flags = SOAPY_SDR_HAS_TIME;

        void *tx_buffs[] = {buffer};
        int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, buffer_size, &flags, tx_time, 400000);
        if (st != (int)buffer_size) {
            std::cerr << "TX failed 256: " << st << std::endl;
        }   

        std::cout << "Block " << block << " transmitted at time " << tx_time << " ns" << std::endl;
    }

    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, txStream);
    SoapySDRDevice_unmake(sdr);

    return EXIT_SUCCESS;
}