# ============================================================
# CMakeLists.txt - VERSÃO FINAL E LIMPA (SEM I2C/SSD1306)
# ============================================================

cmake_minimum_required(VERSION 3.16)

set(PICO_BOARD "pico")

include(external/pico-tflmicro/pico_sdk_import.cmake)

project(pico_inference_project C CXX ASM)

pico_sdk_init()

message(STATUS "🚀 Build limpo (sem I2C) para Pico com TFLM")

# ============================================================
# BIBLIOTECAS EXTERNAS
# ============================================================

add_subdirectory(external/pico-tflmicro)

# ============================================================
# EXECUTÁVEL PRINCIPAL
# ============================================================

add_executable(pico_inference_project
    ${CMAKE_CURRENT_LIST_DIR}/pico_inference_project.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/inference.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/model_data.c
    ${CMAKE_CURRENT_LIST_DIR}/src/image_provider.cpp
)

# ============================================================
# INCLUDES
# ============================================================

target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src
)

# ============================================================
# LINKAGEM DE BIBLIOTECAS
# ============================================================

target_link_libraries(pico_inference_project PRIVATE
    pico-tflmicro
    hardware_uart
    pico_stdio_usb
    pico_stdlib
)

# ============================================================
# CONFIGURAÇÕES FINAIS DO PICO
# ============================================================

pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 1)

pico_add_extra_outputs(pico_inference_project)

message(STATUS "✅ Configuração limpa completa - pronto para build!")
