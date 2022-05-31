cmake_minimum_required(VERSION 3.2)

set(CMAKE_C_OUTPUT_EXTENSION ".rel")
set(CMAKE_C_COMPILER sdcc)
set(CMAKE_SYSTEM_NAME Generic) # No linux target etc

# Prevent default configuration
set(CMAKE_C_FLAGS_INIT "")
set(CMAKE_EXE_LINKER_FLAGS_INIT "")

project(STM8Blink C)
SET(CMAKE_C_FLAGS "-mstm8 --std-c99")
set(SPL_PATH external/spl/)
set(SPL_SRC_PATH ${SPL_PATH}/src)
include_directories(${SPL_PATH}/inc ${SPL_PATH}/conf AFTER)
include_directories(. BEFORE)
# Configure which STM8S we are using
add_definitions(-DSTM8S103)

add_executable(main.ihx
    uart.c
    # I2c_slave_interrupt.c
    # stm8s_it.c
    ${SPL_SRC_PATH}/stm8s_gpio_Init.c
    ${SPL_SRC_PATH}/stm8s_gpio_WriteLow.c
    ${SPL_SRC_PATH}/stm8s_gpio_WriteHigh.c
    ${SPL_SRC_PATH}/stm8s_tim1_GetCounter.c
    ${SPL_SRC_PATH}/stm8s_tim1_Cmd.c
    ${SPL_SRC_PATH}/stm8s_tim4_ClearITPendingBit.c
    ${SPL_SRC_PATH}/stm8s_tim1_TimeBaseInit.c
    ${SPL_SRC_PATH}/stm8s_clk_GetClockFreq.c
    ${SPL_SRC_PATH}/stm8s_clk_HSIPrescalerConfig.c
    ${SPL_SRC_PATH}/stm8s_clk_private.c
    ${SPL_SRC_PATH}/stm8s_uart1_Cmd.c
    ${SPL_SRC_PATH}/stm8s_uart1_Init.c
    ${SPL_SRC_PATH}/stm8s_uart1_ITConfig.c
    ${SPL_SRC_PATH}/stm8s_uart1_SendData8.c
    ${SPL_SRC_PATH}/stm8s_tim1_TimeBaseInit.c
    
)

# Flash targets
add_custom_target(flash COMMAND stm8flash -c stlinkv2 -p stm8s103f3p6 -w main.ihx.ihx)
