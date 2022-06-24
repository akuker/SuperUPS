//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ GPIO Handler ]
//
//  Reads the 2 pushbutton inputs and writes the state of these inputs to
//  I2C registers
//
//---------------------------------------------------------------------------

#include "gpio_handler.h"
#include "i2c_register_data.h"
#include <stdint.h>
#include <stm8s.h>

#define NUM_DEBOUNCE_SAMPLES 4

uint8_t shutdown_button_states[NUM_DEBOUNCE_SAMPLES];
uint8_t auxiliary_button_states[NUM_DEBOUNCE_SAMPLES];

static uint8_t current_idx = 0;

static bool debounce_samples_match(uint8_t *samples)
{
    bool match = TRUE;
    for (int i = 1; i < NUM_DEBOUNCE_SAMPLES; i++)
    {
        if (samples[i - 1] != samples[i])
        {
            match = FALSE;
        }
    }
    return match;
}

void gpios_init()
{

    // There is a pull-up resistor on this input, so a "1" means
    // that the button is NOT pressed
    i2c_register_values[I2C_POWER_BUTTON_STATE] = 1;
    i2c_register_values[I2C_AUX_BUTTON_STATE] = 1;
    GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_IN_FL_NO_IT);
    GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_IN_FL_NO_IT);
}
void gpios_step()
{

    shutdown_button_states[current_idx] = GPIO_ReadInputPin(GPIOC, GPIO_PIN_5);
    auxiliary_button_states[current_idx] = GPIO_ReadInputPin(GPIOC, GPIO_PIN_6);

    // Only update the global values if we're NOT in test mode
    if (i2c_register_values[I2C_TEST_MODE_ENABLE] == 0)
    {
        if (debounce_samples_match(shutdown_button_states))
        {
            i2c_register_values[I2C_POWER_BUTTON_STATE] = shutdown_button_states[0];
        }
        if (debounce_samples_match(auxiliary_button_states))
        {
            i2c_register_values[I2C_AUX_BUTTON_STATE] = auxiliary_button_states[0];
        }
    }

    current_idx = (current_idx + 1) % NUM_DEBOUNCE_SAMPLES;
}
