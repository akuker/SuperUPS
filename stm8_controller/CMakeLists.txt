# Based upon the demo-spl project from:
#   https://github.com/ulikoehler/stm8s-discovery-sdcc-blink

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
set(LIB_CRC_PATH external/libcrc)
set(SPL_SRC_PATH ${SPL_PATH}/src)
set(LIB_CRC_SRC_PATH ${LIB_CRC_PATH}/src)
include_directories(${SPL_PATH}/inc ${LIB_CRC_PATH}/include ${SPL_PATH}/conf AFTER)
include_directories(./inc BEFORE)
# Configure which STM8S we are using
add_definitions(-DSTM8S103)

add_executable(main
    src/main.c
    src/ups_state_machine.c
    src/mosfet_handler.c
    src/i2c_register_data.c
    src/i2c_slave.c
    src/adc_utilities.c
    src/uart_utilities.c
    ${LIB_CRC_SRC_PATH}/crc8.c
    ${SPL_SRC_PATH}/stm8s_adc1_ClearFlag.c
    ${SPL_SRC_PATH}/stm8s_adc1_Cmd.c
    ${SPL_SRC_PATH}/stm8s_adc1_DeInit.c
    ${SPL_SRC_PATH}/stm8s_adc1_Init.c
    ${SPL_SRC_PATH}/stm8s_adc1_GetFlagStatus.c
    ${SPL_SRC_PATH}/stm8s_adc1_ExternalTriggerConfig.c
    ${SPL_SRC_PATH}/stm8s_adc1_StartConversion.c
    ${SPL_SRC_PATH}/stm8s_adc1_PrescalerConfig.c
    ${SPL_SRC_PATH}/stm8s_adc1_ConversionConfig.c
    ${SPL_SRC_PATH}/stm8s_adc1_GetConversionValue.c
    ${SPL_SRC_PATH}/stm8s_adc1_SchmittTriggerConfig.c
    ${SPL_SRC_PATH}/stm8s_gpio_Init.c
    ${SPL_SRC_PATH}/stm8s_gpio_WriteLow.c
    ${SPL_SRC_PATH}/stm8s_gpio_WriteHigh.c
    ${SPL_SRC_PATH}/stm8s_clk_GetClockFreq.c
    ${SPL_SRC_PATH}/stm8s_clk_private.c
    ${SPL_SRC_PATH}/stm8s_clk_PeripheralClockConfig.c
    ${SPL_SRC_PATH}/stm8s_uart1_DeInit.c
    ${SPL_SRC_PATH}/stm8s_uart1_Init.c
    ${SPL_SRC_PATH}/stm8s_uart1_GetFlagStatus.c
    ${SPL_SRC_PATH}/stm8s_uart1_SendData8.c
)

# Flash targets
add_custom_target(flash COMMAND stm8flash -c stlinkv2 -p stm8s103f3p6 -w main.ihx)
