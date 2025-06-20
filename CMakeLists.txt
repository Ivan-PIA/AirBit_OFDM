
cmake_minimum_required(VERSION 3.10)

project(AD9361 CXX)

set(CMAKE_CXX_STANDARD 17)

# Указать текущую директорию
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Подключение OpenMP
find_package(OpenMP REQUIRED)

# Подключение Python3
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter Development NumPy)

# Проверка наличия NumPy
execute_process(COMMAND "${Python3_EXECUTABLE}" -m pip show numpy
    RESULT_VARIABLE has_numpy
    OUTPUT_QUIET ERROR_QUIET)
if(NOT has_numpy EQUAL "0")
    message(FATAL_ERROR "NumPy is required but not installed.")
endif()

# Вывод отладочной информации
message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
message(STATUS "Python3_LIBRARIES: ${Python3_LIBRARIES}")
message(STATUS "Python3_NumPy_INCLUDE_DIRS: ${Python3_NumPy_INCLUDE_DIRS}")

find_package(SoapySDR REQUIRED)

if (SoapySDR_FOUND)
    message(STATUS "SoapySDR_INCLUDE_DIRS: ${SoapySDR_INCLUDE_DIRS}")
    message(STATUS "SoapySDR_LIBRARIES: ${SoapySDR_LIBRARIES}")
    message(STATUS "SoapySDR_DEFINITIONS: ${SoapySDR_DEFINITIONS}")
endif()


# Заголовочные файлы
set(HEADER_FILES
    omp.h
    iio/iio.h
    ../ad9361/ad9361.hpp
    ../log_level/log.hpp
    ../stream/iiostream-common.hpp
    iio/iio-debug.h
    ../src/pocess_buffer/processor.h
    ../QAM/qam_mod.h
    ../QAM/qam_demod.h
    ../Segmenter/segmenter.h
    ../OFDM/ofdm_mod.h
    ../OFDM/ofdm_demod.h
    ../OFDM/freq_offset.hpp
    ../OFDM/sequence.h
    ../File_converter/file_converter.h
    ../src/correlation/time_corr.hpp
    ../OFDM/fft/fft.h
    ../other/plots.h
    ../matplotlibcpp.h
    ../soapy_trx.h
    ../soapy_dev.h
)

# RX файлы
set(RX
    ../log_level/log.cpp
    ../ad9361/ad9361.cpp
    ../src/correlation/time_corr.cpp
    ../src/process_buffer/processor.cpp
    ../OFDM/ofdm_mod.cpp
    ../OFDM/fft/fft.cpp
    ../OFDM/sequence.cpp
    ../OFDM/freq_offset.cpp
    ../other/plots.cpp
    ../File_converter/file_converter.cpp
    ../QAM/qam_mod.cpp
    ../QAM/qam_demod.cpp
    ../Segmenter/segmenter.cpp
    ../OFDM/ofdm_demod.cpp
    ../soapy/soapy_drawplot.cpp
    # ../src/test/tes.cpp
    ../src/test/tx2_plot.cpp
    # ../src/test/rx.cpp
    # /home/ivan/Desktop/OFDM_transceiver/src/test/trx_soapy.cpp
)

# TX файлы
set(TX
    ../log_level/log.cpp
    ../ad9361/ad9361.cpp
    ../OFDM/freq_offset.cpp
    ../src/process_buffer/processor.cpp
    ../QAM/qam_mod.cpp
    ../QAM/qam_demod.cpp
    ../Segmenter/segmenter.cpp
    ../OFDM/ofdm_mod.cpp
    ../OFDM/fft/fft.cpp
    ../OFDM/ofdm_demod.cpp
    ../File_converter/file_converter.cpp
    ../src/correlation/time_corr.cpp
    ../OFDM/sequence.cpp
    ../soapy/soapy_dev.cpp
    # ../soapy/soapy2.cpp
    # ../soapy/soapy_tx.cpp
    # ../other/plots.cpp
    
    ../src/test/tx2.cpp
    
)

# Исполняемые файлы
add_executable(plot ${RX})
add_executable(tx ${TX})

# Указать include директории для RX и TX
target_include_directories(plot PUBLIC ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS} )
target_include_directories(tx PUBLIC ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})

# Линковка Python библиотек
target_link_libraries(plot ${Python3_LIBRARIES} OpenMP::OpenMP_CXX ${SoapySDR_LIBRARIES})
target_link_libraries(tx ${Python3_LIBRARIES} OpenMP::OpenMP_CXX ${SoapySDR_LIBRARIES})

# Линковка libiio
add_library(iio_1x SHARED IMPORTED)
set_target_properties(iio_1x PROPERTIES
    IMPORTED_LOCATION "/home/ivan/libiio/build/libiio.so"
    INTERFACE_INCLUDE_DIRECTORIES "/home/ivan/libiio/include"
)
target_link_libraries(plot iio_1x)
target_link_libraries(tx iio_1x)
# target_link_libraries(rx )
# Вывод сообщения об успешной настройке
message(STATUS "Configuration completed successfully.")

# ################################3
# cmake_minimum_required(VERSION 3.10)  

# project(AD9361 CXX)

# set(CMAKE_CXX_STANDARD 17)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# # include_directories("/usr/include/iio")

# find_package(OpenMP REQUIRED)

# find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter Development NumPy)
# find_package(Python3 COMPONENTS NumPy REQUIRED)
# # Убедиться, что установлен numpy
# execute_process(COMMAND "${Python3_EXECUTABLE}" -m pip show numpy
#     RESULT_VARIABLE has_numpy
#     OUTPUT_QUIET ERROR_QUIET)
# if(NOT has_numpy EQUAL "0")
#     message(FATAL_ERROR "NumPy is required but not installed.")
# endif()


# set(HEADER_FILES
#     omp.h
#     iio/iio.h
#     ../ad9361/ad9361.hpp
#     ../log_level/log.hpp
#     ../stream/iiostream-common.hpp
#     iio/iio-debug.h
#     ../src/pocess_buffer/processor.h
#     ../QAM/qam_mod.h
#     ../QAM/qam_demod.h
#     ../Segmenter/segmenter.h
#     ../OFDM/ofdm_mod.h
#     ../OFDM/ofdm_demod.h
#     ../OFDM/freq_offset.hpp
#     ../OFDM/sequence.h
#     ../File_converter/file_converter.h
#     #../src/correlation/time_corr.hpp
#     ../src/correlation/time_corr.hpp
#     ../OFDM/fft/fft.h
#     ../other/plots.h
#     ../matplotlibcpp.h
    
#     # ad9361.h 
#     #device_api.h
# )


# set(RX
#     ../log_level/log.cpp
#     ../ad9361/ad9361.cpp
#     ../src/correlation/time_corr.cpp
#     # .../src/process_buffer/processor.cpp
#     ../src/process_buffer/processor.cpp
#     ../OFDM/ofdm_mod.cpp
#     ../OFDM/fft/fft.cpp
#     ../OFDM/sequence.cpp
#     ../OFDM/freq_offset.cpp
#     #../stream/iiostream-common.cpp
#     # device_api.c
    
#     ../other/plots.cpp
#     ../File_converter/file_converter.cpp  
#     ../QAM/qam_mod.cpp 
#     ../QAM/qam_demod.cpp 
#     ../Segmenter/segmenter.cpp 
#     ../OFDM/ofdm_mod.cpp 
#     ../OFDM/ofdm_demod.cpp 
    
#     ../src/test/rx.cpp
# )


# # set(TX
# #     ../log_level/log.cpp
# #     ../ad9361/ad9361.cpp
# #     ../src/correlation/time_corr.cpp
# #     # ../stream/iiostream-common.cpp
# #     # device_api.c
# #     ../src/test/tx.cpp

# # )

# set(TX
#     ../log_level/log.cpp
#     ../ad9361/ad9361.cpp
#     ../OFDM/freq_offset.cpp
#     ../src/process_buffer/processor.cpp
#     ../QAM/qam_mod.cpp
#     # /home/ivan/Desktop/OFDM_transceiver/QAM/qam_mod.cpp
#     ../QAM/qam_demod.cpp
#     ../Segmenter/segmenter.cpp
#     ../OFDM/ofdm_mod.cpp
#     ../OFDM/fft/fft.cpp
#     ../OFDM/ofdm_demod.cpp
#     ../File_converter/file_converter.cpp
#     ../src/correlation/time_corr.cpp
#     # ../src/process_buffer/processor.cpp
    
#     # ../src/freq_offset/freq_ofset.cpp
#     ../OFDM/sequence.cpp
#     # /home/ivan/Desktop/OFDM_transceiver/src/freq_offset/freq_ofset.cpp
#     # ../../QAM/qam_demod.cpp
#     # ../../Segmenter/segmenter.cpp
#     # ../../OFDM/ofdm_mod.cpp
#     # ../../OFDM/ofdm_demod.cpp
#     # ../../File_converter/file_converter.cpp
#     # ../stream/iiostream-common.cpp
#     # device_api.c
#     ../src/test/tx2.cpp
#     # ../other/plots.cpp

# )



# # set(TRX
# #     ../log_level/log.cpp
# #     ../ad9361/ad9361.cpp
# #     ../stream/iiostream-common.cpp
# #     # device_api.c
# #     ../src/trx.cpp

# # )

# add_executable(rx ${RX})
# add_executable(tx ${TX})
# # add_executable(trx ${TRX})

# # target_include_directories(tx       
# #                                 PUBLIC /usr/include/python3.10
# #                                 PUBLIC ${Python3_INCLUDE_DIRS}
# #                                 PUBLIC ${Python3_NumPy_INCLUDE_DIRS}
                                    
# #                             )






# target_include_directories(rx       
#                                 PUBLIC /usr/include/python3.10
#                                 PUBLIC ${Python3_INCLUDE_DIRS}
#                                 PUBLIC ${Python3_NumPy_INCLUDE_DIRS}
                                    
#                             )
                            
# target_link_libraries(rx ${Python3_LIBRARIES} python3.10 )
# # target_link_libraries(rx iio)
# target_link_libraries(rx OpenMP::OpenMP_CXX)
# # target_link_libraries(tx iio)
# target_link_libraries(tx OpenMP::OpenMP_CXX)

# add_library(iio_1x SHARED IMPORTED) # or STATIC instead of SHARED
# set_target_properties(iio_1x PROPERTIES
#   IMPORTED_LOCATION "/home/ivan/libiio/build/libiio.so"
#   INTERFACE_INCLUDE_DIRECTORIES "/home/ivan/libiio/include"
# )
# target_link_libraries(rx iio_1x)
# target_link_libraries(tx iio_1x)
# # target_link_libraries(trx iio)