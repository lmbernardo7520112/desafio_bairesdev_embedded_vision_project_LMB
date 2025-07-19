set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_C_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m0plus -mthumb -O2 -fno-exceptions -ffunction-sections -fdata-sections")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-m0plus -mthumb")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections")

set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_VERBOSE_MAKEFILE ON)