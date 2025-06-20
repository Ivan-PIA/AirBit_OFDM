#include "processor.h"



#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"

#include "../../OFDM/sequence.h"
#include "../../OFDM/fft/fft.h"
#include "../../File_converter/file_converter.h"
#include "../../OFDM/freq_offset.hpp"

#include "../../OFDM/ofdm_demod.h"


int process_buffers(std::vector<comp> rx_data){

	OFDM_mod ofdm_mod;
	auto pss = ofdm_mod.mapPSS();

	std::vector<comp> data_offset;
	frequency_correlation(pss, rx_data, 2100000/128 ,data_offset, 2100000);

	// for (int i = 0; i < rx_data.size(); i ++){
	// 	fprintf(file3, "%d\n", (int16_t)rx_data[i].imag());
	// 	fprintf(file4, "%d\n", (int16_t)rx_data[i].real());
	// }
	// fclose(file3);
	// fclose(file4);


	// cool_plot(data_offset, "-", "plot", false);
	// resource_grid(data_offset, 10,6, 128, 32, "rx", false);
	// cool_scatter(data_offset, "plot", false);
	// show_plot();

	OFDM_demod ofdm_demod;
    auto demod_signal = ofdm_demod.demodulate(rx_data);



    QAM_demod qam_demod;
    auto demod_bits = qam_demod.demodulate(demod_signal);
	
	Segmenter segmenter;
    auto demod_bits_m = segmenter.reshape(demod_bits);
    demod_bits_m = segmenter.scramble(demod_bits_m);

    auto data = segmenter.extract_data(demod_bits_m);
    auto flag = segmenter.extract_flag(demod_bits_m);

    if      (flag == 1) bits2string(data);
    else if (flag == 2) bits2file("/home/ivan/Desktop/OFDM_transceiver/src/test", data);
	printf("[%s] [%d ] \n", __func__, __LINE__);

	return 1;
}