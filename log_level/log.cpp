#include "log.hpp"
#include <cstdarg>
#include <cstdio>
#include <fstream>

void show_log(int type_log, const char *format, ... )
{
    va_list args;
    va_start(args, format);
    
    switch (type_log)
    {
    case (CONSOLE):
        vprintf(format, args);
        break;
    case (DEBAG):
        vprintf(format, args);
        break;
    case FILE_LOG: { // Новый случай для записи в файл
        const char *filename = "/home/ivan/SDR_libiio_1.x/resurs_file/log.txt"; // Имя файла для логов
        std::ofstream ofs(filename, std::ios::app); 
        if (ofs.is_open()) {
            
            const int buffer_size = 100000; 
            char buffer[buffer_size];
            vsnprintf(buffer, buffer_size, format, args);
            ofs << buffer; 
            ofs.close(); 
        } else {
            
            fprintf(stderr, "Ошибка: не удалось открыть файл %s для записи.\n", filename);
        }
        break;
    }       
    
    default:
        break;
    }

    va_end(args);
}


