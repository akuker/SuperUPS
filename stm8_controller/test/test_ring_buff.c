//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ SuperUPS main ]
//
//  UART example based upon jukkas example:
//     https://github.com/jukkas/stm8-sdcc-examples
//
//---------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "adc_utilities.h"
#include "gpio_handler.h"
#include "i2c_slave.h"
#include "i2c_register_data.h"
#include "uart_utilities.h"
#include "ups_constants.h"
#include "ups_state_machine.h"
#include "power_control.h"
#include "timer4_utilities.h"

extern u8 i2c_counter;

static const char STATE_DISABLED_STRING[] = "STATE_DISABLED";
static const char STATE_WAIT_OFF_STRING[] = "STATE_WAIT_OFF";
static const char STATE_WAIT_ON_STRING[] = "STATE_WAIT_ON";
static const char STATE_POWERUP_STRING[] = "STATE_POWERUP";
static const char STATE_RUNNING_STRING[] = "STATE_RUNNING";
static const char STATE_FAIL_SHUTDOWN_STRING[] = "STATE_POWER_LOSS_SHUTDOWN";
static const char STATE_FAIL_SHUTDOWN_DELAY_STRING[] = "STATE_POWER_LOSS_SHUTDOWN_DELAY";
static const char STATE_CYCLE_DELAY_STRING[] = "STATE_COMMANDED_POWER_CYCLE_DELAY";

const char *states_strings[] = {
    STATE_DISABLED_STRING,
    STATE_WAIT_OFF_STRING,
    STATE_WAIT_ON_STRING,
    STATE_POWERUP_STRING,
    STATE_RUNNING_STRING,
    STATE_FAIL_SHUTDOWN_STRING,
    STATE_FAIL_SHUTDOWN_DELAY_STRING,
    STATE_CYCLE_DELAY_STRING,
};

void debug_output()
{
    printf("%s ", states_strings[i2c_register_values[I2C_CURENT_STATE]]);
    printf("TEST: %02X ", i2c_register_values[I2C_TEST_MODE_ENABLE] );
    printf("VIN:%02X %d ", i2c_register_values[I2C_ADC_VOLTAGE_IN], i2c_register_values[I2C_ADC_VOLTAGE_IN]);
    printf("VUPS:%02X %d ", i2c_register_values[I2C_ADC_SUPER_CAP_VOLTAGE], i2c_register_values[I2C_ADC_SUPER_CAP_VOLTAGE]);
    printf("VOUT:%02X %d ", i2c_register_values[I2C_ADC_VOLTAGE_OUT], i2c_register_values[I2C_ADC_VOLTAGE_OUT]);
    printf("Shutdown %02X ", i2c_register_values[I2C_POWER_BUTTON_STATE]);
    printf("Aux Btn %02X ", i2c_register_values[I2C_AUX_BUTTON_STATE]);
    printf("Mosfet: %d ", i2c_register_values[I2C_MOSFET_ENABLE]);
    printf("OFF: %02X ", i2c_register_values[I2C_12V_OFF_THRESH]);
    printf("ON: %02X ", i2c_register_values[I2C_12V_ON_THRESH]);
    printf("UPS THR: %02X ", i2c_register_values[I2C_UPS_POWERUP_THRESH]);
    printf("i2c reads: %d\n\r", i2c_counter);
}

#define MASK_BITS(x) ((char)(x & 0xFF))
void test_ringbuffer()
{
    ring_buffer_t test_ringbuffer;
    /* Create and initialize the character buffer */
    ring_buffer_init(&test_ringbuffer);

    // printf("Testing full buffer\n\r");
    // for (uint16_t cnt=0; cnt <  (RING_BUFFER_SIZE-1); cnt++)
    // {
    //     ring_buffer_queue(&test_ringbuffer, MASK_BITS(cnt));
    //     printf("%02X ", MASK_BITS(cnt));
    // }
    // printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));

    // for (uint16_t cnt=0; cnt <  (RING_BUFFER_SIZE-1); cnt++)
    // {
    //     char ret_data;
	// 	if (ring_buffer_dequeue(&test_ringbuffer, &ret_data))
	// 	{
    //         printf("%02X ", MASK_BITS(ret_data));
	// 		if(ret_data != MASK_BITS(cnt)){
    //             printf("[1] Expected to deque %02X but got %02X\n\r", MASK_BITS(cnt), ret_data);
    //             return;
    //         }
	// 	}
    //     else {
    //         printf("[1] Unable to dequeue anything at count %02X\n\r", MASK_BITS(cnt));
    //         return;
    //     }
    // }
    // printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));

    printf("Testing overflow buffer\n\r");
    for (uint16_t cnt=0; cnt < ((2 * RING_BUFFER_SIZE)-1); cnt++)
    {
        ring_buffer_queue(&test_ringbuffer, MASK_BITS(cnt));
        printf("%02X ", MASK_BITS(cnt));
    }
    printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));

    for (uint16_t cnt=RING_BUFFER_SIZE; cnt <  (2 * RING_BUFFER_SIZE)-1; cnt++)
    {
        char ret_data;
		if (ring_buffer_dequeue(&test_ringbuffer, &ret_data))
		{
            printf("%02X ", MASK_BITS(ret_data));
			if(ret_data != cnt){
                printf("[2] Expected to deque %02X but got %02X\n", MASK_BITS(cnt), ret_data);
                return;
            }
		}
        else {
            printf("[2] Unable to dequeue anything at count %02X\n", MASK_BITS(cnt));
            return;
        }
    }
    printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));

    for(int smpl_size = 1; smpl_size < (RING_BUFFER_SIZE); smpl_size++){
        printf("Testing buffer size %d\n\r", smpl_size);
        for (uint16_t cnt=0; cnt < smpl_size; cnt++)
        {
            ring_buffer_queue(&test_ringbuffer, MASK_BITS(cnt));
            // printf("%02X ", MASK_BITS(cnt));
        }
        printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));

        for (uint16_t cnt=0; cnt <  smpl_size; cnt++)
        {
            char ret_data;
            if (ring_buffer_dequeue(&test_ringbuffer, &ret_data))
            {
                // printf("%02X ", MASK_BITS(ret_data));
                if(ret_data != cnt){
                    printf("[3] Expected to deque %02X but got %02X\n", MASK_BITS(cnt), ret_data);
                    return;
                }
            }
            else {
                printf("[3] Unable to dequeue anything at count %02X\n", MASK_BITS(cnt));
                return;
            }
        }
        printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));
    }

    for(int smpl_size = RING_BUFFER_SIZE-1; smpl_size > 0; smpl_size--){
        printf("Testing buffer size %d\n\r", smpl_size);
        for (uint16_t cnt=0; cnt < smpl_size; cnt++)
        {
            ring_buffer_queue(&test_ringbuffer, MASK_BITS(cnt));
            // printf("%02X ", MASK_BITS(cnt));
        }
        printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));

        for (uint16_t cnt=0; cnt <  smpl_size; cnt++)
        {
            char ret_data;
            if (ring_buffer_dequeue(&test_ringbuffer, &ret_data))
            {
                // printf("%02X ", MASK_BITS(ret_data));
                if(ret_data != cnt){
                    printf("[4] Expected to deque %02X but got %02X\n", MASK_BITS(cnt), ret_data);
                    return;
                }
            }
            else {
                printf("[4] Unable to dequeue anything at count %02X\n", MASK_BITS(cnt));
                return;
            }
        }
        printf("   Size: %d IsFull: %d\n\r", ring_buffer_num_items(&test_ringbuffer), ring_buffer_is_full(&test_ringbuffer));
    }

}

static uint16_t counter = 0;

int main(void)
{
    /* Set clock to full speed (16 Mhz) */
    CLK->CKDIVR = 0;

    init_register_data();
    power_control_init();
    uart_init();
    i2c_init();
    ups_init();
    adc_init();
    gpios_init();

    test_ringbuffer();

    /* Enable general interrupts */
    enableInterrupts();
    adc_step();
    debug_output();

    while (1)
    {
        adc_step();
        ups_step();
        pwr_ctrl_step();
        gpios_step();
        counter++;
        if (counter > 999){
            counter = 0;
            debug_output();
        }
        delay_ms(1);
    }
}
