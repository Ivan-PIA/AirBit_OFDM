#ifndef CONFIG_H
#define CONFIG_H

// Параметры OFDM
#define N_FFT 128
#define CP    32
#define G_SUBCAR 55
#define N_PILOTS 12
#define CP_LEN 1 // Normal 0, Extended 1

#define IQ_MODULATION 2 // BPSK 1, QPSK 2, 16QAM 4, 64QAM 6
#define OFDM_SYM_IN_SLOT (CP_LEN == 0 ? 6 : 5)

// Параметры сегментера 
#define MAX_LEN_LINE ((N_FFT-G_SUBCAR-N_PILOTS-1) * IQ_MODULATION * OFDM_SYM_IN_SLOT) // активных 360 поднесущих
#define SEGMENT_NUM_BITS 10
#define USEFUL_BITS 14
#define CRC_BITS 14

#define FS_2 1920000
#define N_CELL_ID 0


#endif //CONFIG_H