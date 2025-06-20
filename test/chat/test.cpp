#include <fstream>
#include <vector>
#include <complex>
#include <string>
#include <cmath>
#include <limits>
#include <unistd.h> // Для функции sleep
#include <omp.h>
#include <thread> // Для многозадачности
#include <atomic> // Для безопасного взаимодействия между потоками
#include <mutex>

#include "../../QAM/qam_mod.h"
#include "../../QAM/qam_demod.h"
#include "../../Segmenter/segmenter.h"
#include "../../OFDM/ofdm_mod.h"
#include "../../OFDM/ofdm_demod.h"
#include "../../OFDM/sequence.h"
#include "../../OFDM/fft/fft.h"
#include "../../File_converter/file_converter.h"
#include "../../OFDM/freq_offset.hpp"
#include "../../other/model_channel.h"
#include "../../other/plots.h"
#include "chat.h"



using cd = std::complex<double>;
namespace plt = matplotlibcpp;


// g++ test.cpp -I/usr/include/python3.10 -lpython3.10 -fopenmp ../../other/plots.cpp ../../other/model_channel.cpp ../../File_converter/file_converter.cpp  ../../QAM/qam_mod.cpp ../../QAM/qam_demod.cpp ../../Segmenter/segmenter.cpp ../../OFDM/ofdm_mod.cpp ../../OFDM/ofdm_demod.cpp ../../OFDM/fft/fft.cpp ../../OFDM/sequence.cpp ../../OFDM/freq_offset.cpp -o test && ./test



std::atomic<bool> newPeerMessageAvailable(false);
std::string peerMessageBuffer;
std::mutex chatHistoryMutex;
std::vector<std::string> chatHistory;

void renderChat(int rows, int cols) {
    clearScreen();
    int maxHistoryLines = rows - 6; // Резервируем место для инструкций и строки ввода
    int startLine = std::max(0, static_cast<int>(chatHistory.size()) - maxHistoryLines);

    // Выводим историю чата
    for (int i = startLine; i < chatHistory.size(); ++i) {
        std::cout << chatHistory[i] << "\n";
    }

    // Оставляем место для строки ввода
    for (int i = 0; i < maxHistoryLines - (chatHistory.size() - startLine); ++i) {
        std::cout << "\n";
    }

    // Вывод инструкций
    std::cout << "=== Чат между двумя SDR ===\n"
              << "1 <сообщение>\n"
              << "2 <путь к файлу>\n"
              << "q - Выйти\n"
              << "=====================\n";
}

void getPeerMessageAsync() {
    // Заглушка: имитация получения сообщения от собеседника в отдельном потоке
    static int counter = 0;
    std::vector<std::string> peerMessages = {
        "Привет! Как дела?",
        "SDR — это круто!",
        "Ты пробовал OFDM модуляцию?"
    };

    while (counter < peerMessages.size()) {
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Симуляция задержки
        peerMessageBuffer = peerMessages[counter++];
        newPeerMessageAvailable.store(true); // Уведомляем о новом сообщении
    }
}

void chatInputAsync() {
    std::string input;
    while (true) {
        std::cout << "↪ ";
        std::getline(std::cin, input);

        if (input == "q") {
            break;
        } else if (input.rfind("1 ", 0) == 0) { // Сообщение с префиксом 1
            std::lock_guard<std::mutex> lock(chatHistoryMutex);
            chatHistory.push_back("Вы: " + input.substr(2));
        } else if (input.rfind("2 ", 0) == 0) { // Отправка файла с префиксом 2
            std::lock_guard<std::mutex> lock(chatHistoryMutex);
            chatHistory.push_back("Вы отправили файл: " + input.substr(2));
        } else { // Просто сообщение
            std::lock_guard<std::mutex> lock(chatHistoryMutex);
            chatHistory.push_back("Вы: " + input);
        }
    }
}

void chatMode() {
    int rows, cols;
    std::thread peerMessageThread(getPeerMessageAsync);
    std::thread userInputThread(chatInputAsync);

    while (true) {
        getTerminalSize(rows, cols);

        // Проверяем, есть ли новое сообщение от собеседника
        if (newPeerMessageAvailable.load()) {
            std::lock_guard<std::mutex> lock(chatHistoryMutex);
            chatHistory.push_back("Собеседник: " + peerMessageBuffer);
            newPeerMessageAvailable.store(false); // Сбрасываем флаг
        }

        renderChat(rows, cols);

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Ожидание между отрисовками
    }

    userInputThread.join();
    peerMessageThread.join();
}

void mainMenu() {
    while (true) {
        clearScreen();
        std::cout << "=== Меню программы ===\n"
                  << "1 - Настройка своей SDR\n"
                  << "2 - Посмотреть настройки передачи\n"
                  << "3 - Перейти в чат\n"
                  << "q - Выйти\n"
                  << "======================\n";

        std::cout << "↪ ";
        char choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == '1') {
            sdrSettings();
        } else if (choice == '2') {
            viewTransmissionSettings();
        } else if (choice == '3') {
            chatMode();
        } else if (choice == 'q') {
            std::cout << "Выход из программы...\n";
            break;
        } else {
            std::cout << "Неверная команда. Попробуйте снова.\n";
            sleep(1);
        }
    }
}

int main() {
    mainMenu();
    return 0;
}