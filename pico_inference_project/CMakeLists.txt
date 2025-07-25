# ============================================================
# CMakeLists.txt - VERSÃO DE DIAGNÓSTICO (OTIMIZAÇÕES REMOVIDAS)
# ============================================================

cmake_minimum_required(VERSION 3.16)

set(PICO_BOARD "pico")
set(PICO_NO_MALLOC 1)

include(external/pico-tflmicro/pico_sdk_import.cmake)

project(pico_inference_project C CXX ASM)

pico_sdk_init()

message(STATUS "🚀 Build configurado para Pico (padrão, sem WiFi) com TensorFlow Lite Micro")

# ============================================================
# BIBLIOTECAS EXTERNAS
# ============================================================

set(PICO_TFLMICRO_ENABLE_TESTS OFF CACHE BOOL "Disable tests" FORCE)
set(PICO_TFLMICRO_ENABLE_EXAMPLES OFF CACHE BOOL "Disable examples" FORCE)

add_subdirectory(external/pico-tflmicro)
add_subdirectory(lib/ssd1306)

# ============================================================
# EXECUTÁVEL PRINCIPAL
# ============================================================

add_executable(pico_inference_project
    ${CMAKE_CURRENT_LIST_DIR}/pico_inference_project.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/inference.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/model_data.c
    ${CMAKE_CURRENT_LIST_DIR}/src/image_provider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306/font8x8_basic.c
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
    ssd1306
    pico_multicore
    hardware_i2c
    hardware_gpio
    hardware_uart
    pico_stdio_usb
    pico_stdio_uart
    pico_stdlib
)

# ============================================================
# DEFINIÇÕES DE COMPILAÇÃO
# ============================================================

target_compile_definitions(pico_inference_project PRIVATE
    CMSIS_NN
    TF_LITE_STATIC_MEMORY
    TF_LITE_DISABLE_X86_NEON
    PICO_NO_MALLOC
    CMSIS_NN_USE_CMSIS_DSP
    ARM_MATH_CM0PLUS
)

# ============================================================
# OTIMIZAÇÕES E CORREÇÃO DE LINKER
# ============================================================

# Mantemos as otimizações de compilação, que são seguras.
target_compile_options(pico_inference_project PRIVATE
    -Os
    -fomit-frame-pointer
    -fno-unroll-loops
    -fno-exceptions
    -ffunction-sections
    -fdata-sections
)

# CRÍTICO: Comentamos as flags de otimização de LINKAGEM que estão
# causando o problema para fins de diagnóstico.
target_link_options(pico_inference_project PRIVATE
    # -flto
    # -Wl,--gc-sections
)

# ============================================================
# CONFIGURAÇÕES FINAIS DO PICO
# ============================================================

pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 1)

pico_set_binary_type(pico_inference_project default)
pico_add_extra_outputs(pico_inference_project)

message(STATUS "✅ Configuração completa - pronto para build!")



