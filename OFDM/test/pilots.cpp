#include "../ofdm_mod.h"
#include "../fft/fft.h"
#include "../sequence.h"
#include <iostream>

// g++ pilots.cpp ../ofdm_mod.cpp ../fft/fft.cpp ../sequence.cpp -o test && ./test

int main(){
    OFDM_mod OFDM_mod;
    auto data = OFDM_mod.data_indices;
    auto data_noPilots = OFDM_mod.data_indices_noPilots;
    auto data_shifted = OFDM_mod.data_indices_shifted;
    auto pilot = OFDM_mod.pilot_indices;
    auto pilot_shifted = OFDM_mod.pilot_indices_shifted;

    std::cout << "Data indices" << std::endl;
    for (auto i : data){
        std::cout << i << " ";
    }
    std::cout << std::endl;

    std::cout << "Data indices ALL" << std::endl;
    for (auto i : data_noPilots){
        std::cout << i << " ";
    }
    std::cout << std::endl;

    std::cout << "Data shifted" << std::endl;
    for (auto i : data_shifted){
        std::cout << i << " ";
    }
    std::cout << std::endl;

    std::cout << "Pilot indices" << std::endl;
    for (auto i : pilot){
        std::cout << i << " ";
    }
    std::cout << std::endl;

    std::cout << "Pilot shifted" << std::endl;
    for (auto i : pilot_shifted){
        std::cout << i << " ";
    }
    std::cout << std::endl;

    return 0;
}