#include "../sequence.h"
#include <iostream>
#include <complex>

// g++ test_sss.cpp ../sequence.cpp -o test && ./test

int main(){
    int cell_id = 0;  // Пример ID соты
    auto sss = generate_sss(cell_id);    

    for(auto s : sss){
        std::cout << s << " ";
    }
    std::cout <<  std::endl;



    return 0;
}