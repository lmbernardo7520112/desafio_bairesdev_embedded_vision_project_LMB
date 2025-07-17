cmake_minimum_required(VERSION 3.16)

# Nome do projeto
project(pico_inference_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define a placa alvo
set(PICO_BOARD pico_w)

# Importa o SDK do Pico
include(pico_sdk_import.cmake)
pico_sdk_init()

# Adiciona subdiretórios corretamente
add_subdirectory(tflm EXCLUDE_FROM_ALL)
add_subdirectory(lib/ssd1306 EXCLUDE_FROM_ALL)

# Define o executável principal
add_executable(pico_inference_project
    pico_inference_project.c
    src/inference.cpp
    src/model_data.cc
)

# Diretórios de include
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/tflm
)

# Bibliotecas necessárias
target_link_libraries(pico_inference_project
    pico_stdlib
    hardware_uart
    tflite_micro
    ssd1306
)

# Configuração do I/O
pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 0)

# Gera UF2, bin, elf etc.
pico_add_extra_outputs(pico_inference_project)
