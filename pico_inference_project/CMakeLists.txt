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
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro)
    message(FATAL_ERROR "TensorFlow Lite Micro não encontrado em external/pico-tflmicro. Clone o repositório: git clone https://github.com/raspberrypi/pico-tflmicro.git external/pico-tflmicro")
endif()
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
# Verifica se wifi_config.h existe
if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/include/wifi_config.h)
    message(FATAL_ERROR "wifi_config.h não encontrado em include/. Crie com #define WIFI_SSID e WIFI_PASS.")
endif()

# Inclui diretórios para headers do projeto, TFLM, e lwIP
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include  # Para wifi_config.h, model_settings.h, inference.h
    ${CMAKE_CURRENT_LIST_DIR}/src      # Para arquivos de origem
    ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src  # Para TensorFlow Lite Micro
    ${PICO_SDK_PATH}/lib/lwip/src/include  # Para lwIP
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
    ${PICO_SDK_PATH}/lib/lwip/src/include/posix
    ${CMAKE_CURRENT_LIST_DIR}  # Para lwipopts.h personalizado
)

# ============================================================
# Definições de compilação
# ============================================================
target_compile_definitions(pico_inference_project PRIVATE
    CYW43_LWIP=1  # Habilita lwIP para Wi-Fi
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1  # Modo thread-safe para Wi-Fi
)

# ============================================================
# Bibliotecas necessárias
# ============================================================
target_link_libraries(pico_inference_project
    pico_stdlib
    pico_stdio_usb
    hardware_uart
    pico-tflmicro
    pico_cyw43_arch_lwip_threadsafe_background  # Wi-Fi + lwIP para UDP
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