// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * libiio - AD9361 IIO streaming example
 *
 * Copyright (C) 2014 IABG mbH
 * Author: Michael Feilen <feilen_at_iabg.de>
 **/



#include "../ad9361/ad9361.hpp"
#include "../log_level/log.hpp"
#include "../src/correlation/time_corr.hpp"
#include "../src/process_buffer/processor.h"

struct iio_context *ctx   = NULL;
struct iio_channel *rx0_i = NULL;
struct iio_channel *rx0_q = NULL;
struct iio_channel *tx0_i = NULL;
struct iio_channel *tx0_q = NULL;
struct iio_buffer  *rxbuf = NULL;
struct iio_buffer  *txbuf = NULL;
struct iio_stream  *rxstream = NULL;
struct iio_stream  *txstream = NULL;

struct iio_channels_mask *rxmask = NULL;
struct iio_channels_mask *txmask = NULL;

static bool stop = false;

void stop_stream(void)
{
	stop = true;
}



/* cleanup and exit */
void shutdown(void)
{
	printf("* Destroying streams\n");
	if (rxstream) {iio_stream_destroy(rxstream); }
	if (txstream) { iio_stream_destroy(txstream); }

	printf("* Destroying buffers\n");
	if (rxbuf) { iio_buffer_destroy(rxbuf); }
	if (txbuf) { iio_buffer_destroy(txbuf); }

	printf("* Destroying channel masks\n");
	if (rxmask) { iio_channels_mask_destroy(rxmask); }
	if (txmask) { iio_channels_mask_destroy(txmask); }

	printf("* Destroying context\n");
	if (ctx) { iio_context_destroy(ctx); }
	exit(0);
}


void handle_sig(int sig)
{
	printf("Waiting for process to finish... Got signal %d\n", sig);
	stop_stream();
}

/* check return value of attr_write function */
static void errchk(int v, const char* what) {
	if (what == "sampling_frequency"){
		show_log(DEBAG, "\n[  errchk v: | %d ] \n",  v);
	}
	
	if (v < 0) { fprintf(stderr, "Error %d writing to channel \"%s\"\nvalue may not be supported.\n", v, what); shutdown(); }
}

/* write attribute: long long int */
static void wr_ch_lli(struct iio_channel *chn, const char* what, long long val)
{	
	const struct iio_attr *attr = iio_channel_find_attr(chn, what);
	// show_log(DEBAG, "\n[  iio_attr_write_longlong: | %d ] \n",  iio_attr_write_longlong(attr, val));
	
	errchk(attr ? iio_attr_write_longlong(attr, val) : -ENOENT, what);
}

/* write attribute: string */
static void wr_ch_str(struct iio_channel *chn, const char* what, const char* str)
{
	const struct iio_attr *attr = iio_channel_find_attr(chn, what);

	errchk(attr ? iio_attr_write_string(attr, str) : -ENOENT, what);
}

/* helper function generating channel names */
static char* get_ch_name(const char* type, int id)
{
	snprintf(tmpstr, sizeof(tmpstr), "%s%d", type, id);
	return tmpstr;
}

/* returns ad9361 phy device */
struct iio_device* get_ad9361_phy(void)
{
	struct iio_device *dev =  iio_context_find_device(ctx, "ad9361-phy");
	IIO_ENSURE(dev && "No ad9361-phy found");
	return dev;
}

/* finds AD9361 streaming IIO devices */
bool get_ad9361_stream_dev(enum iodev d, struct iio_device **dev)
{
	switch (d) {
	case TX: *dev = iio_context_find_device(ctx, "cf-ad9361-dds-core-lpc"); return *dev != NULL;
	case RX: *dev = iio_context_find_device(ctx, "cf-ad9361-lpc");  return *dev != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* finds AD9361 streaming IIO channels */
bool get_ad9361_stream_ch(enum iodev d, struct iio_device *dev, int chid, struct iio_channel **chn)
{
	*chn = iio_device_find_channel(dev, get_ch_name("voltage", chid), d == TX);
	if (!*chn)
		*chn = iio_device_find_channel(dev, get_ch_name("altvoltage", chid), d == TX);
	return *chn != NULL;
}

/* finds AD9361 phy IIO configuration channel with id chid */
bool get_phy_chan(enum iodev d, int chid, struct iio_channel **chn)
{
	switch (d) {
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), false); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("voltage", chid), true);  return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

/* finds AD9361 local oscillator IIO configuration channels */
bool get_lo_chan(enum iodev d, struct iio_channel **chn)
{
	switch (d) {
	 // LO chan is always output, i.e. true
	case RX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 0), true); return *chn != NULL;
	case TX: *chn = iio_device_find_channel(get_ad9361_phy(), get_ch_name("altvoltage", 1), true); return *chn != NULL;
	default: IIO_ENSURE(0); return false;
	}
}

static void rd_ch_long(struct iio_channel *chn, const char* what, long long *val)
{
	const struct iio_attr *attr = iio_channel_find_attr(chn, what);

	errchk(attr ? iio_attr_read_longlong(attr, val) : -ENOENT, what);
}


/* applies streaming configuration through IIO */
bool cfg_ad9361_streaming_ch(struct stream_cfg *cfg, enum iodev type, int chid)
{
	const struct iio_attr *attr,*attr_gain;
	struct iio_channel *chn = NULL;

	// Configure phy and lo channels
	printf("* Acquiring AD9361 phy channel %d\n", chid);
	if (!get_phy_chan(type, chid, &chn)) {	return false; }

	attr = iio_channel_find_attr(chn, "rf_port_select");
	attr_gain = iio_channel_find_attr(chn, "gain_control_mode");

	if (attr)
		errchk(iio_attr_write_string(attr, cfg->rfport), cfg->rfport);

	if (attr_gain)
		errchk(iio_attr_write_string(attr_gain, "manual"), "manual");

	// long long temp = 0;
	// rd_ch_long(chn, "sampling_frequency", &temp);
	// show_log(DEBAG, "\n[ get sampling_frequency | %d ] \n", temp);
	
	show_log(DEBAG, "\n[ rf_bandwidth | %d ] \n", cfg->bw_hz);
	

	wr_ch_lli(chn, "rf_bandwidth",       cfg->bw_hz);

	show_log(DEBAG, "\n[ sampling_frequency | %d ] \n", cfg->fs_hz);
	wr_ch_lli(chn, "sampling_frequency", cfg->fs_hz);
	// wr_ch_lli(chn, "sampling_frequency", 1920000);

	long long temp = 0;
	rd_ch_long(chn, "sampling_frequency", &temp);
	show_log(DEBAG, "\n[ get sampling_frequency | %d ] \n", temp);

	if(type == RX){
		wr_ch_lli(chn, "hardwaregain",RX_GAIN);
		show_log(DEBAG, "\n[ hardwaregain | %d ] \n\n", RX_GAIN);
	}
	else {
		wr_ch_lli(chn, "hardwaregain", TX_GAIN);
	}

	// Configure LO channel
	printf("* Acquiring AD9361 %s lo channel\n", type == TX ? "TX" : "RX");
	if (!get_lo_chan(type, &chn)) { return false; }

	wr_ch_lli(chn, "frequency", cfg->lo_hz);

	//int ret = iio_channel_attr_write(chn,"gain_control_mode", "manual");

	return true;

	//attr_gain =  iio_channel_find_attr(chn, "gain_control_mode");

	// iio_channel_attr_write(chn, "gain_control_mode", "manual");

	// //attr_gain = iio_channel_find_attr(chn, "gain_control_mode");
	// if (type == RX){   
		
    // 	wr_ch_lli(chn, "hardwaregain", 20); // RX gain
	// }

	// else {
	// 	//wr_ch_lli(chn, "powerdown", 1);
	// 	wr_ch_lli(chn, "hardwaregain", 0);
	// }


}


struct iio_device* context_tx(char *ip){
	
	struct iio_device *tx;
	size_t ntx = 0;
	size_t tx_sample_sz;

	struct stream_cfg txcfg;

	int err;

	// TX stream config
	txcfg.bw_hz = MHZ(BW); // 1.5 MHz rf bandwidth
	txcfg.fs_hz = MHZ(FS);   // 2.5 MS/s tx sample rate
	txcfg.lo_hz = MHZ(LO); // 2.5 GHz rf frequency
	txcfg.rfport = "A"; // port A (select for rf freq.)

	printf("* Acquiring IIO context\n");

	IIO_ENSURE((ctx = iio_create_context(NULL, ip)) && "No context");

	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");

	printf("* Acquiring AD9361 streaming devices\n");
	IIO_ENSURE(get_ad9361_stream_dev(TX, &tx) && "No tx dev found");

	printf("* Configuring AD9361 for streaming\n");

	IIO_ENSURE(cfg_ad9361_streaming_ch(&txcfg, TX, 0) && "TX port 0 not found");

	printf("* Initializing AD9361 IIO streaming channels\n");

	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 0, &tx0_i) && "TX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(TX, tx, 1, &tx0_q) && "TX chan q not found");


	txmask = iio_create_channels_mask(iio_device_get_channels_count(tx));
	if (!txmask) {
		fprintf(stderr, "Unable to alloc channels mask\n");
		shutdown();
	}

	printf("* Enabling IIO streaming channels\n");
	iio_channel_enable(tx0_i, txmask);
	iio_channel_enable(tx0_q, txmask);

	printf("* Creating non-cyclic IIO buffers with 1 MiS\n");

	txbuf = iio_device_create_buffer(tx, 0, txmask);
	
	err = iio_err(txbuf);
	if (err) {
		txbuf = NULL;
		// dev_perror(tx, err, "Could not create TX buffer");
		shutdown();
	}
	printf("deb11\n");

	txstream = iio_buffer_create_stream(txbuf, 4, BLOCK_SIZE);
	err = iio_err(txstream);
	if (err) {
		txstream = NULL;
		// dev_perror(tx, iio_err(txstream), "Could not create TX stream");
		shutdown();
	}
	printf("deb12\n");
	return tx;
}


struct iio_device* context_rx(char *ip){
	// Streaming devices

	struct iio_device *rx;

	// RX and TX sample counters
	size_t nrx = 0;


	// RX and TX sample size
	

	// Stream configurations
	struct stream_cfg rxcfg;

	int err;

	// Listen to ctrl+c and IIO_ENSURE
	//signal(SIGINT, handle_sig);

	// RX stream config
	rxcfg.bw_hz = MHZ(BW);   // 2 MHz rf bandwidth
	rxcfg.fs_hz = MHZ(FS); // 2.5 MS/s rx sample rate
	rxcfg.lo_hz = MHZ(LO); // 2.5 GHz rf frequency
	rxcfg.rfport = "A_BALANCED"; // port A (select for rf freq.)



	printf("* Acquiring IIO context\n");
	// if (argc == 1) {
	// 	IIO_ENSURE((ctx = iio_create_context(NULL, NULL)) && "No context");
	// }
	show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);

	IIO_ENSURE((ctx = iio_create_context(NULL, ip)) && "No context");

	IIO_ENSURE(iio_context_get_devices_count(ctx) > 0 && "No devices");

	printf("* Acquiring AD9361 streaming devices\n");

	IIO_ENSURE(get_ad9361_stream_dev(RX, &rx) && "No rx dev found");

	printf("* Configuring AD9361 for streaming\n");
	IIO_ENSURE(cfg_ad9361_streaming_ch(&rxcfg, RX, 0) && "RX port 0 not found");


	printf("* Initializing AD9361 IIO streaming channels\n");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 0, &rx0_i) && "RX chan i not found");
	IIO_ENSURE(get_ad9361_stream_ch(RX, rx, 1, &rx0_q) && "RX chan q not found");


	rxmask = iio_create_channels_mask(iio_device_get_channels_count(rx));
	if (!rxmask) {
		fprintf(stderr, "Unable to alloc channels mask\n");
		shutdown();
	}


	printf("* Enabling IIO streaming channels\n");
	iio_channel_enable(rx0_i, rxmask);
	iio_channel_enable(rx0_q, rxmask);


	printf("* Creating non-cyclic IIO buffers with 1 MiS\n");
	rxbuf = iio_device_create_buffer(rx, 0, rxmask);
	err = iio_err(rxbuf);
	if (err) {
		rxbuf = NULL;
		// dev_perror(rx, err, "Could not create RX buffer");
		shutdown();
	}

	rxstream = iio_buffer_create_stream(rxbuf, 4, BLOCK_SIZE);
	// show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);
	return rx;

}

struct iio_device *initialize_device_rx(char *ip, size_t &rx_sample_sz, const struct iio_block *&rxblock, struct iio_device *&rx, struct iio_stream *&rxstream)
{	
	const struct iio_device *dev;
	const struct iio_context *ctx;
	// show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);

    rx = context_rx(ip);
	// show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);
    rx_sample_sz = iio_device_get_sample_size(rx, rxmask);
	// show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);
	show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);
	dev = iio_channel_get_device(rx0_i);
	ctx = iio_device_get_context(dev);


    // rxblock = iio_stream_get_next_block(rxstream);

    return rx;
}

struct iio_device *initialize_device_tx(char *ip, size_t &tx_sample_sz, struct iio_device *&tx)
{	
	const struct iio_device *dev;
	const struct iio_context *ctx;

    tx = context_tx(ip);

	show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);
    tx_sample_sz = iio_device_get_sample_size(tx, txmask);

	dev = iio_channel_get_device(tx0_i);
	ctx = iio_device_get_context(dev);

    

    return tx;
}

std::vector<comp> process_rx_block(const struct iio_block *rxblock, size_t rx_sample_sz)
{
    int16_t *p_dat, *p_end;
    ptrdiff_t p_inc = rx_sample_sz;
    
	std::vector<comp> rx_data;

	// show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);

    p_end = (int16_t*)iio_block_end(rxblock);
    for (p_dat = (int16_t*)iio_block_first(rxblock, rx0_i); p_dat < p_end; 
         p_dat += p_inc / sizeof(*p_dat)) 
    {
        /* Example: swap I and Q */
        int16_t i = p_dat[0];
        int16_t q = p_dat[1];

        p_dat[0] = q;
        p_dat[1] = i;

		rx_data.push_back(comp(i, q));
        // fprintf(file3, "%d\n", q);
        // fprintf(file4, "%d\n", i);
    }
	// show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);
	return rx_data;
}

std::vector<comp> read_from_block(char *ip, int count)
{
    size_t rx_sample_sz;
    struct iio_device *rx;
    const struct iio_block *rxblock;
    // struct iio_stream *rxstream;

	std::vector<comp> temp;
	std::vector<comp> rx_data;
	
    rx = initialize_device_rx(ip, rx_sample_sz, rxblock, rx, rxstream);
	int i = 0;
	
	while(i < count)
	{	
		
		rxblock = iio_stream_get_next_block(rxstream);
		temp = process_rx_block(rxblock, rx_sample_sz);
		rx_data.insert(rx_data.end(), temp.begin(), temp.end());
		i++;

	}


        
	return rx_data;
}



void write_to_file(std::vector<comp> data){
	FILE *file3, *file4;

	file3  = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/all_rx_imag.txt", "w");
    file4 = fopen("/home/ivan/SDR_libiio_1.x/resurs_file/all_rx_real.txt", "w");
	for (int i = 0; i < data.size(); i ++){
		fprintf(file3, "%d\n", (int16_t)data[i].imag());
		fprintf(file4, "%d\n", (int16_t)data[i].real());
	}
	fclose(file3);
	fclose(file4);

}




std::vector<comp> flatten(const std::vector<std::vector<comp>>& vec) {
    std::vector<comp> flat_vector;

    for (const auto& inner_vec : vec) {
        flat_vector.insert(flat_vector.end(), inner_vec.begin(), inner_vec.end());
    }

    return flat_vector;
}

std::vector<comp> read_from_block_real(char *ip)
{	
	std::mutex buffer_mutex;
    size_t rx_sample_sz;
    struct iio_device *rx;
    const struct iio_block *rxblock;
    std::vector<comp> temp;
    std::vector<comp> rx_data;

	std::vector<comp> data_with_pss;
	std::vector<comp> all_buffer;
    // Инициализация устройства
    rx = initialize_device_rx(ip, rx_sample_sz, rxblock, rx, rxstream);

    int block_counter = 0;
    std::vector<std::vector<comp>> buffer_set;  // Для хранения трех буферов

	
	show_log(CONSOLE, "\t*** START RECIVING ***\n");
    
    while (!stop)
    {
        rxblock = iio_stream_get_next_block(rxstream);
        temp = process_rx_block(rxblock, rx_sample_sz);

        // Добавляем текущий буфер данных в общий
        rx_data.insert(rx_data.end(), temp.begin(), temp.end());

        // Сохраняем буфер в буферный набор
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            buffer_set.push_back(temp);
        }

        block_counter++;
		// show_log(DEBAG, "\n[ %s | %d ] \n\n", __func__, __LINE__);
        // Если накоплено три буфера, запускаем функцию свертки в отдельном потоке
        if (block_counter == 6)
        {
            // Асинхронный вызов функции свертки
            std::future<std::pair<int, std::vector<comp>>> conv_and_concat_result = std::async(std::launch::async, [&buffer_set]() {
                std::vector<comp> combined_data;
                std::vector<comp> concatenated_buffer;

    
                // Склеиваем три буфера в один для свертки
                for (const auto& buf : buffer_set) {
                    combined_data.insert(combined_data.end(), buf.begin(), buf.end());
                }

                // Выполняем свертку на склеенном буфере
                int convolution_result = _conv(combined_data);

                
                concatenated_buffer.insert(concatenated_buffer.end(), combined_data.begin(), combined_data.end());

                return std::make_pair(convolution_result, concatenated_buffer);
            });
			

            // Ожидаем завершения свертки
            // int result = conv_result.get();
			auto [convolution_result, concatenated_buffer] = conv_and_concat_result.get();

            // Если результат свертки больше 1, останавливаем прием данных
			
            if (convolution_result > 5)
            {
				{
					std::lock_guard<std::mutex> lock(buffer_mutex);
					data_with_pss = flatten(buffer_set);
				}
                stop = true;
				show_log(CONSOLE, "\n\t  *** PSS FOUND ***\n\n");
				all_buffer = concatenated_buffer;
            }

            // Очищаем буферный набор и увеличиваем счетчик блоков
            {
                std::lock_guard<std::mutex> lock(buffer_mutex);
                buffer_set.clear();
            }
            block_counter = 0;
        }
    }
	write_to_file(rx_data);

    return data_with_pss;
}

std::vector<comp> read_from_block_real1(char *ip) {
    std::mutex buffer_mutex;
    size_t rx_sample_sz;
    struct iio_device *rx;
    const struct iio_block *rxblock;
    std::vector<comp> temp;
    std::vector<comp> rx_data;

    std::vector<comp> data_with_pss;
    std::vector<comp> all_buffer;
    bool pss_found = false;

    // Инициализация устройства
    rx = initialize_device_rx(ip, rx_sample_sz, rxblock, rx, rxstream);

    int block_counter = 0;
    std::vector<std::vector<comp>> buffer_set;  // Для хранения трех буферов

    show_log(CONSOLE, "\t*** START RECEIVING ***\n");

    while (!stop) {
        rxblock = iio_stream_get_next_block(rxstream);
        temp = process_rx_block(rxblock, rx_sample_sz);

        // Добавляем текущий буфер данных в общий
        rx_data.insert(rx_data.end(), temp.begin(), temp.end());

        // Сохраняем буфер в буферный набор
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            buffer_set.push_back(temp);
        }

        block_counter++;

        // Если накоплено шесть буферов, запускаем функцию свертки в отдельном потоке
        if (block_counter == 6) {
            // Асинхронный вызов функции свертки
            std::future<std::pair<int, std::vector<comp>>> conv_and_concat_result = std::async(std::launch::async, [&buffer_set]() {
                std::vector<comp> combined_data;
                std::vector<comp> concatenated_buffer;

                // Склеиваем буферы
                for (const auto& buf : buffer_set) {
                    combined_data.insert(combined_data.end(), buf.begin(), buf.end());
                }

                // Выполняем свертку на склеенном буфере
                int convolution_result = _conv(combined_data);

                concatenated_buffer.insert(concatenated_buffer.end(), combined_data.begin(), combined_data.end());

                return std::make_pair(convolution_result, concatenated_buffer);
            });

            auto [convolution_result, concatenated_buffer] = conv_and_concat_result.get();

            // Если результат свертки больше 6, устанавливаем флаг pss_found
            if (convolution_result > 6) {
                {
                    std::lock_guard<std::mutex> lock(buffer_mutex);
                    data_with_pss = flatten(buffer_set);
                }
                pss_found = true;
                show_log(CONSOLE, "\n\t  *** PSS FOUND ***\n\n");

                // Запуск обработки с найденными данными в отдельном потоке
                std::thread([data_with_pss]() {
                    process_buffers(data_with_pss); // Ваша функция обработки PSS данных
                }).detach();

                all_buffer = concatenated_buffer;
            }

            // Очищаем буферный набор и увеличиваем счетчик блоков
            {
                std::lock_guard<std::mutex> lock(buffer_mutex);
                buffer_set.clear();
            }
            block_counter = 0;
        }
    }

    write_to_file(rx_data);

    return data_with_pss;
}

std::vector<comp> read_from_block_real2(char *ip) {
    std::mutex buffer_mutex;
    size_t rx_sample_sz;
    struct iio_device *rx;
    const struct iio_block *rxblock;
    std::vector<comp> temp;
    std::vector<comp> rx_data;
    std::vector<comp> signal_buffer;  // Буферы, где найден сигнал
    int block_counter = 0;
    std::vector<std::vector<comp>> buffer_set;  // Для хранения буферов

    // Флаг для остановки при вводе 'q'
    std::atomic<bool> stop(false);
	int count_pss = 0 ;
    // Поток для отслеживания пользовательского ввода
    std::thread input_thread([&stop]() {
        char input;
        while (std::cin >> input) {
            if (input == 'q') {
                stop = true;
                std::cout << "\n\t*** STOPPING RECEPTION ***\n";
                break;
            }
        }
    });

    // Инициализация устройства
    rx = initialize_device_rx(ip, rx_sample_sz, rxblock, rx, rxstream);

    show_log(CONSOLE, "\t*** START RECEIVING ***\n");

    while (!stop) {
        // Получаем следующий блок данных
        rxblock = iio_stream_get_next_block(rxstream);
        temp = process_rx_block(rxblock, rx_sample_sz);

        // Добавляем текущий буфер данных в общий массив всех данных
        rx_data.insert(rx_data.end(), temp.begin(), temp.end());

        // Сохраняем буфер в буферный набор
        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            buffer_set.push_back(temp);
        }

        block_counter++;

        // Если накоплено шесть буферов, выполняем свертку для обнаружения сигнала
        if (block_counter == 1) {
            // Асинхронный вызов свертки для обнаружения сигнала
            std::future<std::pair<int, std::vector<comp>>> conv_and_concat_result = std::async(std::launch::async, [&buffer_set]() {
                std::vector<comp> combined_data;

                // Склеиваем буферы для свертки
                for (const auto& buf : buffer_set) {
                    combined_data.insert(combined_data.end(), buf.begin(), buf.end());
                }

                int convolution_result = _conv(combined_data);

                return std::make_pair(convolution_result, combined_data);
            });

            auto [convolution_result, concatenated_buffer] = conv_and_concat_result.get();

            // Если сигнал найден, сохраняем буферы с сигналом
            if (convolution_result > 3) {
                show_log(CONSOLE, "\n\t  *** PSS FOUND ***\n\n");
				count_pss ++;
                {
                    std::lock_guard<std::mutex> lock(buffer_mutex);
                    // Добавляем текущие буферы с сигналом в общий буфер
                    for (const auto& buf : buffer_set) {
                        signal_buffer.insert(signal_buffer.end(), buf.begin(), buf.end());
                    }
                }
            }

            // Сбрасываем буферный набор для следующей итерации
            {
                std::lock_guard<std::mutex> lock(buffer_mutex);
                buffer_set.clear();
            }
            block_counter = 0;
        }
    }

    // Ожидание завершения потока ввода
    input_thread.join();

    // Запись только тех данных, где найден сигнал
    write_to_file(signal_buffer);
	std::cout << "Count-PSS: " << count_pss << std::endl;
    return signal_buffer; // Возвращаем объединенный буфер с сигналами
}



// void send_ofdm_symbols(char *ip, std::vector<comp> &ofdm_symbols) {
//     const int symbols_per_slot = 6;
//     const int slots_per_buffer = 7;
//     const int symbols_per_buffer = symbols_per_slot * slots_per_buffer;

//     size_t tx_sample_sz;
//     struct iio_device *tx;
//     const struct iio_block *txblock;

//     tx = initialize_device_tx(ip, tx_sample_sz, tx);

//     int total_symbols = ofdm_symbols.size();
//     int full_buffers_count = total_symbols / symbols_per_buffer;
//     int remaining_symbols = total_symbols % symbols_per_buffer;

//     int symbol_index = 0;

//     for (int i = 0; i < full_buffers_count; i++) {
//         std::vector<comp> buffer(ofdm_symbols.begin() + symbol_index,
//                                  ofdm_symbols.begin() + symbol_index + symbols_per_buffer);

//         txblock = iio_stream_get_next_block(txstream); // Получаем следующий блок для передачи
//         process_tx_block(tx_sample_sz, buffer);        // Передаем полный буфер
//         symbol_index += symbols_per_buffer;
//     }

    
//     if (remaining_symbols > 0) {
//         std::vector<comp> last_buffer(ofdm_symbols.begin() + symbol_index,
//                                       ofdm_symbols.begin() + symbol_index + remaining_symbols);

//         txblock = iio_stream_get_next_block(txstream); // Получаем следующий блок для передачи
//         process_tx_block(tx_sample_sz, last_buffer);   // Передаем последний неполный буфер
//     }
// }

// std::mutex mtx;
// std::condition_variable cv;
// std::atomic<int> temp_count(0);
// std::vector<comp> rx_data;
// // bool stop = false;

// int process_data()
// {
//     while (true)
//     {
//         std::unique_lock<std::mutex> lock(mtx);
//         cv.wait(lock, [] { return temp_count.load() >= 3 || stop; });

//         if (stop) break;

//         // Вызываем вашу функцию свертки
//         int result = convolve(rx_data);

//         if (result > 1)
//         {
//             stop = true;
// 			return 0;
//         }

//         // Сбрасываем счетчик и очищаем rx_data для следующего цикла
//         temp_count = 0;
//         rx_data.clear();
//     }
// }

// std::vector<comp> read_from_block_real(char *ip)
// {
//     size_t rx_sample_sz;
//     struct iio_device *rx;
//     const struct iio_block *rxblock;
//     // struct iio_stream *rxstream;

//     rx = initialize_device_rx(ip, rx_sample_sz, rxblock, rx, rxstream);
    
//     // Запускаем поток для обработки данных
//     std::thread processing_thread(process_data);

//     while (!stop)
//     {  
//         rxblock = iio_stream_get_next_block(rxstream);
//         std::vector<comp> temp = process_rx_block(rxblock, rx_sample_sz);
        
//         // Защита от одновременного доступа
//         {
//             std::lock_guard<std::mutex> lock(mtx);
//             rx_data.insert(rx_data.end(), temp.begin(), temp.end());
//             temp_count++;
//         }
        
//         // Уведомляем поток обработки данных
//         cv.notify_one();
//     }
    
//     // Ждем завершения обработки
//     if (processing_thread.joinable())
//     {
//         processing_thread.join();
//     }

//     return rx_data; // Вернуть данные, если нужно
// }



void process_tx_block(const struct iio_block *txblock, size_t tx_sample_sz, std::vector<comp> samples)
{
    int16_t *p_dat, *p_end;
    ptrdiff_t p_inc = tx_sample_sz;
    
	

    p_end = (int16_t*)iio_block_end(txblock);
	
	int i = 0;
    for (p_dat = (int16_t*)iio_block_first(txblock, tx0_i); p_dat < p_end; 
         p_dat += p_inc / sizeof(*p_dat)) 
    {
		if(i > samples.size()){
			break;
			// i = 0;
		}
		// std::cout << "samples: " << samples[i].real() << ", " << samples[i].imag() << std::endl;
		p_dat[0] = (int16_t)samples[i].real(); /* Real (I) */
		p_dat[1] = (int16_t)samples[i].imag(); /* Imag (Q) */
		i++;

    }

}
void write_to_block(char *ip, std::vector<comp> tx_data, int count)
{
	const int symbols_per_slot = 6;
    const int slots_per_buffer = 7;
    const int symbols_per_buffer = symbols_per_slot * slots_per_buffer * (N_FFT+CP); // 42 

    size_t tx_sample_sz;
    struct iio_device *tx;
    const struct iio_block *txblock;

    tx = initialize_device_tx(ip, tx_sample_sz, tx);

    int total_symbols = tx_data.size();
    int full_buffers_count = total_symbols / symbols_per_buffer;
    int remaining_symbols = total_symbols % symbols_per_buffer;

	int i = 0;
	int symbol_index = 0;
	int all_sample = 0;
	// txblock = iio_stream_get_next_block(txstream);
	cd complex_value(1200.0, 1800.0);
	vector<cd> buffer_temp(BLOCK_SIZE,complex_value);





	for (int i = 0; i < full_buffers_count; i++) {
		std::vector<comp> buffer(tx_data.begin() + symbol_index,
								tx_data.begin() + symbol_index + symbols_per_buffer);
		
		 // Получаем следующий блок для передачи
		process_tx_block(txblock,tx_sample_sz, buffer);        // Передаем полный буфер
		symbol_index += symbols_per_buffer;
		all_sample += buffer.size();

		sleep(10);
	}

	
	
	if (remaining_symbols > 0) {
        std::vector<comp> last_buffer(tx_data.begin() + symbol_index,
                                      tx_data.begin() + symbol_index + remaining_symbols);

       //txblock = iio_stream_get_next_block(txstream); // Получаем следующий блок для передачи
        process_tx_block(txblock,tx_sample_sz, last_buffer);   // Передаем последний неполный буфер
		std::cout << "all sample: " << all_sample + last_buffer.size() <<std::endl;
		// sleep(10);
    }
	
	// //std::cout << "samples: " << tx_data[0].real() << ", " << tx_data[0].imag() << std::endl;
	txblock = iio_stream_get_next_block(txstream);
	
    // txblock = iio_stream_get_next_block(txstream);
	// while(i < count)
	// { 
	// 	// 
	// 	process_tx_block( tx_sample_sz, tx_data); //txblock,
	// 	//txblock = iio_stream_get_next_block(txstream);
	// 	i++;
	// }
	
	// txblock = iio_stream_get_next_block(txstream);
	
}




void write_to_block2(char *ip, std::vector<comp> tx_data, int count)
{
    size_t tx_sample_sz;
    struct iio_device *tx;
    const struct iio_block *txblock;

    tx = initialize_device_tx(ip, tx_sample_sz, tx);

	int h = 0;

	//std::cout << "samples: " << tx_data[0].real() << ", " << tx_data[0].imag() << std::endl;
	// txblock = iio_stream_get_next_block(txstream);

	cd complex_value(1200.0, 1800.0);
	vector<cd> buffer_temp(BLOCK_SIZE,complex_value);

	// while(h < 100)
	// {	 txblock = iio_stream_get_next_block(txstream);
	// 	// txblock = iio_stream_get_next_block(txstream);
	// 	process_tx_block(txblock, tx_sample_sz, buffer_temp);
	// 	//txblock = iio_stream_get_next_block(txstream);
	// 	printf("%d - buff tx\n", h);
	// 	this_thread::sleep_for(chrono::milliseconds(1));
	// 	h++;
	// }
	
	int i = 0;
	while(i < count)
	{	 txblock = iio_stream_get_next_block(txstream);
		// txblock = iio_stream_get_next_block(txstream);
		process_tx_block(txblock, tx_sample_sz, tx_data);
		//txblock = iio_stream_get_next_block(txstream);
		printf("%d - buff tx\n", i);
		// this_thread::sleep_for(chrono::milliseconds(1));
		i++;
	}
	
	txblock = iio_stream_get_next_block(txstream);
	
}


// int main (int argc, char **argv)
// {
// 	struct iio_device *rx = context_rx(argc,argv);
// 	struct iio_device *tx = context_tx(argc,argv);
//     size_t rx_sample_sz,tx_sample_sz;

//     //rxmask = iio_create_channels_mask(iio_device_get_channels_count(rx));
//     //rx_sample_sz = iio_device_get_sample_size(rx, rxmask);
// 	rx_sample_sz = iio_device_get_sample_size(rx, rxmask);
// 	tx_sample_sz = iio_device_get_sample_size(tx, txmask);

// 	printf("* Starting IO streaming (press CTRL+C to cancel)\n");
	
// 	stream(rx_sample_sz, tx_sample_sz, BLOCK_SIZE,
// 	      rxstream, txstream, rx0_i, tx0_i);
	
// 	shutdown();

// 	return 0;
// }