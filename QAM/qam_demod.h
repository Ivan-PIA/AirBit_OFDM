#ifndef QAM_DEMOD_H
#define QAM_DEMOD_H

#include <vector>
#include <complex>
#include <cmath>

#include <bitset>
#include <map>

#include "qam_mod.h"

class QAM_demod {
public:
    QAM_demod(int modulation = IQ_MODULATION);

    std::vector<uint8_t> demodulate(const std::vector<cd>& receivedSignal);
    std::vector<std::vector<double>> softDecision(const std::vector<cd>& receivedSignal);

private:
    std::vector<std::complex<double>> constellation;

    void generateConstellation(int modulation);
    std::vector<double> calculat_softDecision(const std::complex<double>& point);
};

#endif // QAM_DEMOD_H
