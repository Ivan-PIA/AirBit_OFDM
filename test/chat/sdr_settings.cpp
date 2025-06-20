#include <iostream>
#include <unistd.h> 

void sdrSettings() {
    std::cout << "Настройка SDR...\n";
    sleep(1); // Заглушка для процесса настройки
    std::cout << "Настройки завершены.\n";
    sleep(1);
}
