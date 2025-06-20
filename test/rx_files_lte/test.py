import numpy as np
import matplotlib.pyplot as plt


def load_complex_numbers(filename):
    data = np.loadtxt(filename, dtype=complex)
    return data

filename = "/home/ivan/Desktop/OFDM_transceiver/test/rx_files_lte/rx_file_complex_fs_1.92.txt"
complex_array = load_complex_numbers(filename)

# Вывод массива для проверки
#print(complex_array)

# ml.resource_grid(complex_array, 5)
plt.plot(abs(complex_array))
plt.show()