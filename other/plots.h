#ifndef COOL_PLOT_H
#define COOL_PLOT_H

#include <vector>
#include <complex>
#include <string>
#include <numeric>
#include "../config.h"

#include "../matplotlibcpp.h"

namespace plt = matplotlibcpp;
using cd = std::complex<double>;

// Функция для отображения графика с комплексными числами
void cool_plot(const std::vector<cd>& data, 
               std::string title = "", 
               std::string vid = "-", 
               bool show_plot = false);

// Функция для отображения графика с одним вектором чисел (y)
template <typename T>
void cool_plot(const std::vector<T>& input, 
               std::string title = "", 
               std::string vid = "-", 
               bool show_plot = false);


void cool_scatter(const std::vector<cd>& x, const std::string title = "", bool show_plot = false);
void cool_scatter2(const std::vector<std::complex<double>>& data, const std::string& title, bool show_plot);

void spectrogramAndScatterPlot(const std::vector<std::complex<double>>& data, const std::string& title, bool show_plot);

void spectrogram_plot(const std::vector<cd>& input, const std::string& title = "", size_t FFT_Size = N_FFT, bool show_plot = false);

void spectrogram_plot_subplot(const std::vector<std::complex<double>>& input, const std::string& title, size_t FFT_Size, bool show_plot);
void spectrogram_plot_work(const std::vector<std::complex<double>>& input, const std::string& title, size_t FFT_Size, bool show_plot);

void show_plot();

#endif  // COOL_PLOT_H