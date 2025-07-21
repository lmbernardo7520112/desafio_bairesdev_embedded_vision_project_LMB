
# ============================================================
# VERSÃO AJUSTADA PARA USO COM SUBMÓDULO pico-tflmicro
# ============================================================

cmake_minimum_required(VERSION 3.16)

include("$ENV{PICO_SDK_PATH}/pico_sdk_init.cmake")

project(pico_inference_project C CXX ASM)

pico_sdk_init()

# Define o seu bootloader customizado
set(PICO_BOOT_STAGE2_BINARY ${CMAKE_CURRENT_LIST_DIR}/boot_stage2.bin)

# Adiciona o submódulo pico-tflmicro
add_subdirectory(external/pico-tflmicro)

# Biblioteca de display SSD1306 (se usada)
add_subdirectory(lib/ssd1306)

# Arquivos principais do seu projeto
add_executable(pico_inference_project
    pico_inference_project.c
    src/inference.cpp
    src/model_data.cc
)

target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(pico_inference_project PRIVATE
    pico_stdlib
    hardware_uart
    pico-tflmicro       # <- biblioteca fornecida pelo submódulo
    ssd1306             # <- biblioteca de display
)

pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 0)

pico_add_extra_outputs(pico_inference_project)
