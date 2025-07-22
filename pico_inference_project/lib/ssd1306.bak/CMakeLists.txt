# ====================================================================
# CMakeLists.txt - lib/ssd1306
# ====================================================================

add_library(ssd1306 ssd1306.c)

# Inclui os headers locais da lib e também os do SDK do Pico
target_include_directories(ssd1306 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# Linka com as bibliotecas padrão do SDK (para stdio, gpio, etc)
target_link_libraries(ssd1306
    pico_stdlib        # ← fornece pico/stdlib.h
    hardware_i2c       # ← se você usa I2C no display
)

