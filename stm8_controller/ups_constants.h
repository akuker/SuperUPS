#pragma once
#include <stdint.h>

#define R1 100
#define R2 200

#define VCONV(x) (x * R2 / (R1+R2))
#define VCONV_DIG(x) VCONV(x)*256/2.56

#define GET_ON_THRESH uint8_t(VCONV_DIG(ON_THRESH_DESIRED))
#define GET_POWERUP_THRESH uint8_t(VCONV_DIG(POWERUP_THRESH_DESIRED))
#define GET_OFF_THRESH uint8_t(VCONV_DIG(OFF_THRESH_DESIRED))
#define GET_SHUTDOWN_THRESH uint8_t(VCONV_DIG(SHUTDOWN_THRESH_DESIRED))

#define ERROR_CRC 0xFF
#define ERROR_SIZE 0xFE
#define ERROR_UNINITIALIZED 0xFD
#define ERROR_SUCCESS 0

#define MAX_SAMPLES 8

typedef enum {
    STATE_DISABLED = (uint8_t)0,
    STATE_WAIT_OFF = (uint8_t)1,
    STATE_WAIT_ON = (uint8_t)2,
    STATE_POWERUP = (uint8_t)3,
    STATE_RUNNING = (uint8_t)4,
    STATE_FAIL_SHUTDOWN = (uint8_t)5,
    STATE_FAIL_SHUTDOWN_DELAY = (uint8_t)6,
    STATE_CYCLE_DELAY = (uint8_t)7,
} ups_state_type;


#define VAL_MOSFET i2c_register_values[MOSFET]
#define VAL_VIN_HIGH i2c_register_values[VIN_HIGH]
#define VAL_VUPS_HIGH i2c_register_values[VUPS_HIGH]
#define VAL_ON_THRESH i2c_register_values[ON_THRESH]
#define VAL_OFF_THRESH i2c_register_values[OFF_THRESH]
#define VAL_COUNTDOWN i2c_register_values[COUNTDOWN]
#define VAL_STATE i2c_register_values[STATE]
#define VAL_CYCLE_DELAY i2c_register_values[CYCLE_DELAY]
#define VAL_FAIL_SHUTDOWN_DELAY i2c_register_values[FAIL_SHUTDOWN_DELAY]
#define VAL_RUN_COUNTER i2c_register_values[RUN_COUNTER]
#define VAL_POWERUP_THRESH i2c_register_values[POWERUP_THRESH]
