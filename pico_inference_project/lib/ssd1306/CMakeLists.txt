add_library(ssd1306 STATIC
    ssd1306.c
)

target_include_directories(ssd1306 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ssd1306
    pico_stdlib
    hardware_i2c
)

# ⚠️ Adiciona flags corretas para compilar no modo Thumb-1 (Cortex-M0+)
target_compile_options(ssd1306 PRIVATE
    -mcpu=cortex-m0plus -mthumb
)

