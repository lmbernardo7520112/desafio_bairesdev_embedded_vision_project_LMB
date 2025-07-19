
cmake_minimum_required(VERSION 3.16)

# Inclui o SDK ANTES de tudo. Isso carrega as funções do Pico.
include("$ENV{PICO_SDK_PATH}/pico_sdk_init.cmake")

# Define o projeto. O pico_sdk_init() abaixo vai configurar os compiladores.
project(pico_inference_project C CXX ASM)

# Agora inicializa o SDK. Ele vai detectar a toolchain ARM
# e aplicar as flags de compilação bare-metal necessárias.
pico_sdk_init()

# --- A PARTIR DAQUI, TUDO IGUAL ---

# Define o bootloader customizado.
set(PICO_BOOT_STAGE2_BINARY ${CMAKE_CURRENT_LIST_DIR}/boot_stage2.bin)

# Subdiretórios com bibliotecas
add_subdirectory(tflm)
add_subdirectory(lib/ssd1306)

# Arquivos do executável principal
add_executable(pico_inference_project
    pico_inference_project.c
    src/inference.cpp
    src/model_data.cc
)

# Includes do projeto
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}          # Adiciona a raiz do projeto
    ${CMAKE_CURRENT_LIST_DIR}/src      # Adiciona o diretório src
    ${CMAKE_CURRENT_LIST_DIR}/include  # Mantém o diretório include
    ${CMAKE_CURRENT_LIST_DIR}/tflm     # Mantém o diretório tflm
    ${CMAKE_CURRENT_LIST_DIR}/lib      # Adiciona o diretório lib
)

# Linkagem
target_link_libraries(pico_inference_project
    pico_stdlib
    hardware_uart
    tflite_micro
    ssd1306
    pico_cyw43_arch_lwip_threadsafe_background
)

# STDIO
pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 0)

# Geração dos binários
pico_add_extra_outputs(pico_inference_project)