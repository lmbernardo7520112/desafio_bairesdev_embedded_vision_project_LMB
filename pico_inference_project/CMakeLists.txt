
# ====================================================================
# VERSÃO FINAL E CORRETA - CMakeLists.txt (Principal)
# ====================================================================
cmake_minimum_required(VERSION 3.16)

# Inclui o SDK do Pico e suas funções
include("$ENV{PICO_SDK_PATH}/pico_sdk_init.cmake")

# Define o projeto principal
project(pico_inference_project C CXX ASM)

# Inicializa a toolchain ARM e as configurações do Pico
pico_sdk_init()

# Define o seu bootloader customizado
set(PICO_BOOT_STAGE2_BINARY ${CMAKE_CURRENT_LIST_DIR}/boot_stage2.bin)

# Adiciona os subdiretórios que contêm as bibliotecas
add_subdirectory(tflm)
add_subdirectory(lib/ssd1306)

# Cria o executável final com seus arquivos fonte
add_executable(pico_inference_project
    pico_inference_project.c
    src/inference.cpp
    src/model_data.cc
)

# Define os diretórios de include APENAS do seu código.
# As bibliotecas (tflm, ssd1306) gerenciam seus próprios includes.
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Linka seu executável com as bibliotecas necessárias
target_link_libraries(pico_inference_project PRIVATE
    pico_stdlib
    hardware_uart
    tflite_micro  # <-- Biblioteca criada pelo tflm/CMakeLists.txt
    ssd1306       # <-- Biblioteca criada por lib/ssd1306/CMakeLists.txt
)

# Habilita a saída de dados pela UART
pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 0)

# Gera os arquivos finais (.uf2, .elf, etc.)
pico_add_extra_outputs(pico_inference_project)
