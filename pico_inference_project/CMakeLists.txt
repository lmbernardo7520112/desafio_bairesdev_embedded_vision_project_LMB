cmake_minimum_required(VERSION 3.13)

# Importa SDK do Pico a partir da variável de ambiente PICO_SDK_PATH
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Define a placa como Pico W
set(PICO_BOARD pico_w)

# Nome do projeto e padrões de compilação
project(pico_inference_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o SDK do Pico
pico_sdk_init()

# ============================================================
# Define pico-tflmicro library manually
# ============================================================
file(GLOB PICO_TFLMICRO_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src/*.c"
)
add_library(pico-tflmicro STATIC ${PICO_TFLMICRO_SOURCES})
target_include_directories(pico-tflmicro PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src
)
target_link_libraries(pico-tflmicro PRIVATE pico_stdlib)

# ============================================================
# Arquivos do projeto
# ============================================================
add_executable(pico_inference_project
    pico_inference_project.cpp
    src/image_provider.cpp
    src/inference.cpp
    src/peripherals.c
    src/model_data.c
)

# Define o nome do binário de saída
set_target_properties(pico_inference_project PROPERTIES OUTPUT_NAME pico_inference_project)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# ============================================================
# Includes
# ============================================================
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include # For lwipopts.h, inference.h, model_settings.h
    ${CMAKE_CURRENT_LIST_DIR}/src # For image_provider.h, model_data.h
    ${CMAKE_CURRENT_LIST_DIR}/external/pico-tflmicro/src # Path to pico-tflmicro includes
    ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_arch/include
    ${PICO_SDK_PATH}/lib/cyw43-driver/src
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
    ${PICO_SDK_PATH}/lib/lwip/src/include/posix
    ${PICO_SDK_PATH}/src/rp2_common/pico_cyw43_driver/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include
    ${PICO_SDK_PATH}/src/common/pico_base/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_async_context/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_rand/include
)

# ============================================================
# Definições de compilação
# ============================================================
target_compile_definitions(pico_inference_project PRIVATE
    CYW43_LWIP=1
    PICO_CYW43_ARCH_THREADSAFE_BACKGROUND=1
    LWIP_PROVIDE_ERRNO=1
    TF_LITE_STATIC_MEMORY=1 # Enable static memory for TensorFlow Lite Micro
)

# ============================================================
# Bibliotecas necessárias
# ============================================================
target_link_libraries(pico_inference_project
    pico_stdlib
    pico_lwip
    pico_lwip_arch
    pico_cyw43_arch_lwip_threadsafe_background
    pico_rand
    pico-tflmicro # Link TensorFlow Lite Micro library
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