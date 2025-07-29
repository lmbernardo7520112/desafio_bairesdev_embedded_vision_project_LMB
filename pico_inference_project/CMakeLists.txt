# ============================================================
# CMakeLists.txt - VERSÃO FINAL E LIMPA (SEM I2C/SSD1306)
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

# Adiciona o executável PRINCIPAL e os arquivos de código-fonte
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
    include
    src
    external/pico-tflmicro/src
)

# Vincula as bibliotecas necessárias (sem I2C, sem display)
target_link_libraries(pico_inference_project
    pico_stdlib
    pico_stdio_usb
    hardware_uart
    pico-tflmicro
)

# Gera UF2 e bin
pico_add_extra_outputs(pico_inference_project)
