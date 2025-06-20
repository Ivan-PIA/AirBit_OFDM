#include <iostream>
#include <vector>
#include <complex> 
#include <iterator>
#include <fstream>
#include <cmath>



// g++ plot.cpp -o l -lsfml-graphics -lsfml-window -lsfml-syste

using namespace std;


// need test
// vector<complex<double>> zadoff_chu(int N = 1, int u = 25, bool PSS = false);

//need test
vector<complex<double>> pss_on_carrier(int Nfft);


// working 
// return array convolve for pss and data.
int convolve(vector<complex<double>>x);

// need test 
vector<complex<double>> convolve2(vector<complex<double>>x, vector<complex<double>>h);



int detect_pss(const vector<complex<double>>& data, double threshold);

int _conv(const std::vector<std::complex<double>>& y1);