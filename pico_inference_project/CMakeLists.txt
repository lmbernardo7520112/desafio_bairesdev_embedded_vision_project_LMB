cmake_minimum_required(VERSION 3.13)

# Projeto e configurações
project(pico_inference_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SDK do Pico
set(PICO_BOARD pico_w)
include(pico_sdk_import.cmake)
pico_sdk_init()

# Fonte principal do programa
add_executable(pico_inference_project
    pico_inference_project.c
    src/inference.c
)

# Includes do projeto
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}
)

# TFLM (TensorFlow Lite for Microcontrollers)
add_subdirectory(tflm)

target_link_libraries(pico_inference_project
    pico_stdlib
    hardware_dma
    hardware_timer
    hardware_clocks
    tflite_micro
)

# UART ligado, USB desligado
pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 0)

# Arquivos de saída (UF2, etc)
pico_add_extra_outputs(pico_inference_project)
