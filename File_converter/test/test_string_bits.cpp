#include "../file_converter.h"

int main() {
    auto bits = string2bits("Hello, World!");
    bits2string(bits);

    return 0;
}