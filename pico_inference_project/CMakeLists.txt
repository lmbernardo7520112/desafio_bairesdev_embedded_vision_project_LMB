# ============================================================
# CMakeLists.txt - Pico W (UDP Wi-Fi) + TensorFlow Lite Micro
# ============================================================

cmake_minimum_required(VERSION 3.13)

# Importa SDK do Pico a partir da variável de ambiente PICO_SDK_PATH
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Nome do projeto e padrões de compilação
project(pico_inference_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o SDK do Pico
pico_sdk_init()

# ============================================================
# Dependências externas
# ============================================================
# Adiciona biblioteca do TensorFlow Lite Micro adaptada para Pico
add_subdirectory(external/pico-tflmicro EXCLUDE_FROM_ALL)

# ============================================================
# Arquivos do projeto
# ============================================================
add_executable(pico_inference_project
    pico_inference_project.cpp
    src/inference.cpp
    src/image_provider.cpp
    src/model_data.c
)

set_target_properties(pico_inference_project PROPERTIES OUTPUT_NAME pico_inference_project)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# ============================================================
# Includes
# ============================================================
# Inclui diretórios para encontrar headers do projeto e do TFLM
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    src
    external/pico-tflmicro/src
)

# Garante que o lwipopts.h otimizado para UDP seja usado
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}        # local onde está o lwipopts.h
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
    ${PICO_SDK_PATH}/lib/lwip/src/include/posix
)

# ============================================================
# Definições de compilação (opcional para otimização UDP)
# ============================================================
target_compile_definitions(pico_inference_project PRIVATE
    CYW43_LWIP=1
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1
)

# ============================================================
# Bibliotecas necessárias
# ============================================================
target_link_libraries(pico_inference_project
    pico_stdlib
    pico_stdio_usb
    hardware_uart
    pico-tflmicro
    pico_cyw43_arch_lwip_threadsafe_background  # Wi-Fi + lwIP para UDP rápido
)

# ============================================================
# Saída USB e UART
# ============================================================
pico_enable_stdio_usb(pico_inference_project 1)
pico_enable_stdio_uart(pico_inference_project 0)

# ============================================================
# Geração de binários UF2
# ============================================================
pico_add_extra_outputs(pico_inference_project)
