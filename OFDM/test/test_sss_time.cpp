#include "../sequence.h"
#include <iostream>
#include <chrono>

// g++ test_sss_time.cpp ../sequence.cpp -o test && ./test

int main(){

    auto start_correlate = std::chrono::high_resolution_clock::now();

    for (int cell_id = 0; cell_id < 168; ++cell_id){
        auto sss = generate_sss(cell_id);
    }
    
    auto end_correlate = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_correlate = end_correlate - start_correlate;

    std::cout << "SSS gen: " << duration_correlate.count() << " seconds." << std::endl;
    // SSS gen: 0.00126513 seconds.


    return 0;
}