cmake_minimum_required(VERSION 3.16)

project(pico_inference_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w)
set(PICO_BOOT_STAGE2_BINARY /home/lmbernardo/temp_boot_build/build/boot2_w25q080.bin)


# SDK path
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
endif()

include(pico_sdk_import.cmake)
pico_sdk_init()

# Subdiretórios
add_subdirectory(tflm)
add_subdirectory(lib/ssd1306)

# Executável
add_executable(pico_inference_project
    pico_inference_project.c
    src/inference.cpp
    src/model_data.cc
)

# Includes
target_include_directories(pico_inference_project PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/tflm
    ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306
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

# Output
pico_add_extra_outputs(pico_inference_project)



