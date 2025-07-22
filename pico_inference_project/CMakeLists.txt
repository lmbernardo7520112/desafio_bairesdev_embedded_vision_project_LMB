
# ============================================================
# VERSÃO AJUSTADA PARA USO COM SUBMÓDULO pico-tflmicro
# ============================================================

cmake_minimum_required(VERSION 3.16)

include("$ENV{PICO_SDK_PATH}/pico_sdk_init.cmake")

project(pico_inference_project C CXX ASM)

pico_sdk_init()

# Define bootloader customizado (pré-gerado ou compilado)
set(PICO_BOOT_STAGE2_BINARY ${CMAKE_CURRENT_LIST_DIR}/boot_stage2.bin)

# Adiciona a subpasta da biblioteca TFLM com CMSIS-NN
add_subdirectory(external/pico-tflmicro)

# Biblioteca SSD1306
add_subdirectory(lib/ssd1306)

# Executável principal
add_executable(pico_inference_project
    pico_inference_project.cpp
    src/inference.cpp
    src/model_data.cc
    src/image_provider.cpp
)

# Includes necessários para o TFLM + seu projeto
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src
    ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src/tensorflow
    ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src/third_party
)

# Linkagem de bibliotecas
target_link_libraries(pico_inference_project PRIVATE
    pico_stdlib
    hardware_i2c
    hardware_gpio
    hardware_uart
    pico-tflmicro
    ssd1306
    -Wl,--wrap=malloc
    -Wl,--wrap=free
)

# ⚠️ Habilita CMSIS-NN e memória estática do TFLM
target_compile_definitions(pico_inference_project PRIVATE
    CMSIS_NN
    TF_LITE_STATIC_MEMORY
    PICO_NO_MALLOC
)

# Otimizações de tamanho
target_compile_options(pico_inference_project PRIVATE
    -Os
    -fdata-sections
    -ffunction-sections
)

target_link_options(pico_inference_project PRIVATE
    -Wl,--gc-sections
)

# Configurações específicas da Raspberry Pi Pico
pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 0)
pico_set_binary_type(pico_inference_project no_load)
pico_add_extra_outputs(pico_inference_project)

# Desativa testes do pico-tflmicro se não forem usados
set(PICO_TFLMICRO_ENABLE_TESTS OFF CACHE BOOL "Disable tests" FORCE)

