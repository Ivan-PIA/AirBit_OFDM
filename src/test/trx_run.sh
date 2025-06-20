
# rm /home/ivan/Desktop/OFDM_transceiver/src/test/арбуз_арбуз.jpeg
# cd /home/ivan/Desktop/OFDM_transceiver/build
# cmake ..
# make


# # ./tx /home/ivan/Desktop/OFDM_transceiver/test/test_channel/test_file_in/арбуз_арбуз.jpeg
# # ./tx /home/ivan/Desktop/OFDM_transceiver/test/test_channel/test_file_in/file.jpg
# ./tx /home/ivan/Desktop/OFDM_transceiver/test/test_channel/test_file_in/tenor-google.gif


# # xdg-open /home/ivan/Desktop/OFDM_transceiver/src/test/арбуз_арбуз.jpeg
# # xdg-open /home/ivan/Desktop/OFDM_transceiver/src/test/file.jpg
# xdg-open /home/ivan/Desktop/OFDM_transceiver/src/test/tenor-google.gif

#!/bin/bash

# Папка, откуда передаём
TX_DIR="/home/ivan/Desktop/OFDM_transceiver/test/test_channel/test_file_in"

# Папка, откуда удаляем оригинал
DEL_DIR="/home/ivan/Desktop/OFDM_transceiver/src/test"

# Ассоциация номеров с именами файлов
case "$1" in
  1)
    FILENAME="арбуз_арбуз.jpeg"
    ;;
  2)
    FILENAME="file.jpg"
    ;;
  3)
    FILENAME="tenor-google.gif"
    ;;
  *)
    echo "Использование: $0 [1|2|3]"
    exit 1
    ;;
esac

# Полные пути
TX_FILE="$TX_DIR/$FILENAME"
DEL_FILE="$DEL_DIR/$FILENAME"

# Проверка существования файла для передачи
if [[ ! -f "$TX_FILE" ]]; then
  echo "Файл для передачи не найден: $TX_FILE"
  exit 1
fi

# Проверка существования файла для удаления
if [[ ! -f "$DEL_FILE" ]]; then
  echo "Файл для удаления не найден: $DEL_FILE"
else
  echo "Удаляю: $DEL_FILE"
  rm -v "$DEL_FILE"
fi

cd /home/ivan/Desktop/OFDM_transceiver/build
cmake ..
make

# Передача файла
echo "Передача файла: $TX_FILE"
/home/ivan/Desktop/OFDM_transceiver/build/tx "$TX_FILE"

xdg-open "$DEL_FILE"

