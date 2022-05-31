#include "i2c_register_data.h"
#include "ups_constants.h"

volatile uint8_t i2c_register_values[] =
{
    0x0,        // reserved
    0x1,        // reserved
    0x0,        // vin - low (reserved)
    0x0,        // vin - high
    0x0,        // vups - low (reserved)
    0x0,        // vups - high
    0x0,        // mosfet switch state
    ON_THRESH,  // on threshold
    OFF_THRESH, // off threshold
    0x0,        // countdown in 100ms units
    STATE_POWERUP,   // state
    0x0,        // cycle delay
    0x0,        // fail-shutdown delay
    0x0,        // run counter
    POWERUP_THRESH, // powerup threshold
    SHUTDOWN_THRESH, // software shutdown threshold
    R1/100,     // R1 resistor value
    R2/100,     // R2 resistor value
};