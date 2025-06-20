import numpy as np
import matplotlib.pyplot as plt
import mylib as ml

def load_complex_numbers(filename):
    data = np.loadtxt(filename, dtype=complex)
    return data

filename = "out.txt"
complex_array = load_complex_numbers(filename)

# Вывод массива для проверки
#print(complex_array)

# ml.resource_grid(complex_array, 5)
plt.plot(complex_array)
plt.show()