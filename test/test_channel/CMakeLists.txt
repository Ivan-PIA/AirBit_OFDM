cmake_minimum_required(VERSION 3.10)

# Название проекта
project(test)

# Установить стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Флаги компиляции
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# Указать корневую директорию проекта
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})

# Указать директории с заголовочными файлами
include_directories(
    ${PROJECT_ROOT}/../../QAM
    ${PROJECT_ROOT}/../../Segmenter
    ${PROJECT_ROOT}/../../OFDM
    ${PROJECT_ROOT}/../../OFDM/fft
    ${PROJECT_ROOT}/../../File_converter
    ${PROJECT_ROOT}/../../other
)

# Указать исходные файлы
set(SOURCE_FILES
    test.cpp
    ../../QAM/qam_mod.cpp
    ../../QAM/qam_demod.cpp
    ../../Segmenter/segmenter.cpp
    ../../OFDM/ofdm_mod.cpp
    ../../OFDM/ofdm_demod.cpp
    ../../OFDM/fft/fft.cpp
    ../../OFDM/sequence.cpp
    ../../OFDM/freq_offset.cpp
    ../../File_converter/file_converter.cpp
    ../../other/model_channel.cpp
    ../../other/plots.cpp
)

# Добавить исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# # Устанавливаем рабочую директорию для исполняемого файла
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
#     VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Копировать все файлы из test_file_in в build/test_file_in
file(COPY ${CMAKE_SOURCE_DIR}/test_file_in DESTINATION ${CMAKE_BINARY_DIR})


# Линковка с библиотекой Python (путь нужно настроить, если библиотека находится в нестандартном месте)
find_package(Python3 COMPONENTS Development REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Python3_LIBRARIES})
