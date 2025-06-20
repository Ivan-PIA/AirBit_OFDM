#ifndef CHAT_H
#define CHAT_H

    #include <iostream>
    #include <sys/ioctl.h> // Для определения размера терминала

    void sdrSettings();
    void viewTransmissionSettings();
    void clearScreen() { std::cout << "\033[2J\033[H"; } // ANSI escape sequence для очистки экрана
    
    void getTerminalSize(int &rows, int &cols) {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        rows = w.ws_row;
        cols = w.ws_col;
    }

#endif