cmake_minimum_required(VERSION 3.16)

# ======================== FIXES CRÍTICOS ============================
# Usa as ferramentas corretas da toolchain ARM
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_FLAGS "")  # ⚠️ Evita erro de interpretação incorreta

# Força uso de objdump e objcopy da toolchain, não do sistema
set(CMAKE_OBJDUMP /home/lmbernardo/.pico-sdk/toolchain/14_2_Rel1/bin/arm-none-eabi-objdump)
set(CMAKE_OBJCOPY /home/lmbernardo/.pico-sdk/toolchain/14_2_Rel1/bin/arm-none-eabi-objcopy)
# ===================================================================

# ======================== CONFIGURAÇÃO DO PROJETO ============================
project(pico_inference_project C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w)

# ======================== SDK DO RASPBERRY PI PICO ============================
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
endif()

include(pico_sdk_import.cmake)
pico_sdk_init()

# ======================== FIX BOOT ASM ============================
# Corrige erro do arquivo assembly do boot_stage2
set_source_files_properties(
    ${PICO_SDK_PATH}/src/rp2040/boot_stage2/compile_time_choice.S
    PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -mcpu=cortex-m0plus -mthumb -Wa,--no-warn"
)
# ===================================================================

# ======================== SUBDIRETÓRIOS EXTERNOS ============================
add_subdirectory(tflm)
add_subdirectory(lib/ssd1306)

# ======================== EXECUTÁVEL PRINCIPAL ============================
add_executable(pico_inference_project
    pico_inference_project.c
    src/inference.cpp
    src/model_data.cc
)

# ======================== INCLUDE PATHS ============================
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/tflm
    ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306
)

# ======================== BIBLIOTECAS ============================
target_link_libraries(pico_inference_project
    pico_stdlib
    hardware_uart
    tflite_micro
    ssd1306
    pico_cyw43_arch_lwip_threadsafe_background
)

# ======================== STDIO CONFIG ============================
pico_enable_stdio_uart(pico_inference_project 1)
pico_enable_stdio_usb(pico_inference_project 0)

# ======================== SAÍDAS (UF2, ELF, BIN) ============================
pico_add_extra_outputs(pico_inference_project)
