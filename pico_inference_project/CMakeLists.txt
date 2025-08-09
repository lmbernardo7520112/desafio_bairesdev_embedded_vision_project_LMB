# ============================================================
# CMakeLists.txt - Pico W (UDP Wi-Fi) + TensorFlow Lite Micro
# ============================================================

cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_inference_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o SDK do Pico
pico_sdk_init()

# Adiciona subdiretório do pico-tflmicro (dependência do projeto)
add_subdirectory(external/pico-tflmicro EXCLUDE_FROM_ALL)

# Arquivos do projeto (sem I2C/SSD1306)
add_executable(pico_inference_project
    pico_inference_project.cpp
    src/inference.cpp
    src/image_provider.cpp
    src/model_data.c
)

set_target_properties(pico_inference_project PROPERTIES OUTPUT_NAME pico_inference_project)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Inclui os diretórios com headers
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    src
    external/pico-tflmicro/src
)

# Vincula bibliotecas necessárias
target_link_libraries(pico_inference_project
    pico_stdlib
    pico_stdio_usb
    hardware_uart
    pico-tflmicro
    pico_cyw43_arch_lwip_threadsafe_background  # Wi-Fi + lwIP para UDP
)

# Habilita saída USB e UART (opcional, apenas para debug)
pico_enable_stdio_usb(pico_inference_project 1)
pico_enable_stdio_uart(pico_inference_project 1)

# Gera UF2 e binários
pico_add_extra_outputs(pico_inference_project)
