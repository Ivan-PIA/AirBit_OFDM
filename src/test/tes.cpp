#include <vector>
#include <iostream>
#include <stdio.h>

#include "../../matplotlibcpp.h"


namespace plt = matplotlibcpp;

int main(){
    std::vector<double> x = {1, 2, 3, 4};
    std::vector<double> y = {1, 4, 9, 16};
    printf("E");

    plt::figure_size(1100, 600);

    plt::subplot(2,1,1);
    plt::plot(x, y);
    plt::grid(true);
    plt::subplot(2,1,2);

    plt::plot(y, x);
    
    plt::grid(true);
    plt::show();
}