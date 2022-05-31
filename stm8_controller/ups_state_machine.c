#include "ups_constants.h"
#include "i2c_register_data.h"
#include <stm8s_adc1.h>

inline void mosfetOff()
{
// #ifdef DCDC_INSTEAD_OF_MOSFET
//     // to turn the DCDC off, pull down the enable
//     PORTB &= (~PIN_MOSFET);
//     DDRB |= PIN_MOSFET;
// #else
//     PORTB |= PIN_MOSFET;
// #endif
}

inline void mosfetOn()
{
// #ifdef DCDC_INSTEAD_OF_MOSFET
//     // to turn the DCDC off, float the enable
//     DDRB &= (~PIN_MOSFET);
// #else
//     PORTB &= (~PIN_MOSFET);
// #endif
}

void handleRegMosfet()
{
    if (i2c_register_values[MOSFET]) {
        // low will turn the mosfet on
        // mosfetOn()

#ifdef SLOW_START_MOSFET
        // Slow-start the mosfet.
        // Otherwise, the sudden current inrush will cause the dc/dc
        // converter to trip into overcurrent mode and shut off.
        uint8_t i;
        for (i=0; i<200; i++) {
            mosfetOn();
            delayMicroseconds(3);
            mosfetOff();
            delayMicroseconds(3);
        }
        for (i=0; i<200; i++) {
            mosfetOn();
            delayMicroseconds(6);
            mosfetOff();
            delayMicroseconds(3);
        }
#endif
        mosfetOn();
    } else {
        // high will turn the mosfet off
        mosfetOff();
    }
}

// https://circuitdigest.com/microcontroller-projects/adc-on-stm8s-using-c-compiler-reading-multiple-adc-values-and-displaying-on-lcd
uint16_t ADC_Read(ADC1_Channel_TypeDef ADC_Channel_Number)
{
    uint16_t result = 0;
    ADC1_DeInit();
    ADC1_Init(ADC1_CONVERSIONMODE_CONTINUOUS,
              ADC_Channel_Number,
              ADC1_PRESSEL_FCPU_D18,
              ADC1_EXTTRIG_TIM,
              DISABLE,
              ADC1_ALIGN_RIGHT,
              ADC1_SCHMITTTRIG_ALL,
              DISABLE);
    ADC1_Cmd(ENABLE);
    ADC1_StartConversion();
    while (ADC1_GetFlagStatus(ADC1_FLAG_EOC) == FALSE);
    result = ADC1_GetConversionValue();
    ADC1_ClearFlag(ADC1_FLAG_EOC);
    ADC1_DeInit();
    return result;
}

void ups_init() {

    mosfetOff();
// #ifndef DCDC_INSTEAD_OF_MOSFET
//     // when using the mosfet, pin direction is always output
//     DDRB |= PIN_MOSFET;
// #endif

//     sample_ptr = 0;
//     num_samples = 0;

//     reg_position = 0;

//     receive_error = ERROR_UNINITIALIZED;

//     VAL_RUN_COUNTER = 0;

    // ADC1_DeInit();

    // ADC1_Init(ADC1_CONVERSIONMODE_CONTINUOUS,
    // ADC1_CHANNEL_3,
    // ADC1_PRESSEL_FCPU_D18,
    // ADC1_EXTTRIG_TIM,
    // DISABLE,
    // ADC1_ALIGN_RIGHT,
    // ADC1_SCHMITTTRIG_ALL,
    // DISABLE);
    // ADC1_Cmd(ENABLE);
    // ADC1_StartConversion();

CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE); //Enable Peripheral Clock for ADC
// GPIO_Init (GPIOD, GPIO_PIN_2, GPIO_MODE_IN_FL_IT);
// GPIO_Init (GPIOD, GPIO_PIN_3, GPIO_MODE_IN_FL_IT);


    // while(ADC1_GetFlagStatus(ADC1_FLAG_EOC) == FALSE);
    // result = ADC1_GetConversionValue();
    // ADC1_ClearFlag(ADC1_FLAG_EOC);
    // ADC1_DeInit();

    // itoa(result);

}

void ups_step() {

    // vin_samples[sample_ptr] = readADC(3);
    // vups_samples[sample_ptr] = readADC(2);
    // sample_ptr ++;
    // if (sample_ptr >= MAX_SAMPLES) {
    //     sample_ptr = 0;
    // }
    // num_samples ++;
    // if (num_samples > MAX_SAMPLES) {
    //     num_samples = MAX_SAMPLES;
    // }

    // VAL_VIN_HIGH = computeAverage(vin_samples);
    // VAL_VUPS_HIGH = computeAverage(vups_samples);

    // if (num_samples < MAX_SAMPLES) {
    //     return;
    // }

    switch (VAL_STATE) {
        case STATE_DISABLED:
            // state machine is disabled -- do nothing
            break;

        case STATE_WAIT_OFF:
            if (VAL_VIN_HIGH <= VAL_OFF_THRESH) {
                VAL_STATE = STATE_WAIT_ON;
            }
            break;

        case STATE_WAIT_ON:
            if (VAL_VIN_HIGH >= VAL_ON_THRESH) {
                VAL_STATE = STATE_POWERUP;
            }
            break;

        case STATE_FAIL_SHUTDOWN:
        case STATE_POWERUP:
            if ((VAL_VIN_HIGH >= VAL_ON_THRESH) && (VAL_VUPS_HIGH >= VAL_POWERUP_THRESH)) {
                VAL_COUNTDOWN = 0;
                VAL_MOSFET = 1;
                handleRegMosfet();
                VAL_STATE = STATE_RUNNING;
            }
            break;

        case STATE_RUNNING:
            if ((VAL_VIN_HIGH <= VAL_OFF_THRESH) && (VAL_VUPS_HIGH <= VAL_OFF_THRESH)) {
                VAL_MOSFET = 0;
                handleRegMosfet();
                VAL_FAIL_SHUTDOWN_DELAY = 10;
                VAL_STATE = STATE_FAIL_SHUTDOWN_DELAY;
            }
            if (VAL_COUNTDOWN > 0) {
                if ((VAL_RUN_COUNTER%10)==0) {
                    VAL_COUNTDOWN --;
                }
                if (VAL_COUNTDOWN == 0) {
                    // turn off the mosfet and wait for powercycle before
                    // turning back on.
                    VAL_CYCLE_DELAY = 10;
                    VAL_STATE = STATE_CYCLE_DELAY;
                    VAL_MOSFET = 0;
                    handleRegMosfet();
                }
            }
            break;

        case STATE_FAIL_SHUTDOWN_DELAY:
            // shutdown due to power failure; delay so the pi can safely power cycle
            if (VAL_FAIL_SHUTDOWN_DELAY > 0) {
                VAL_FAIL_SHUTDOWN_DELAY--;
            }
            if (VAL_FAIL_SHUTDOWN_DELAY==0) {
                VAL_STATE = STATE_FAIL_SHUTDOWN;
            }

        case STATE_CYCLE_DELAY:
            // shutdown due to power-off request; delay so the pi can safely power cycle
            if (VAL_CYCLE_DELAY>0) {
                VAL_CYCLE_DELAY--;
            }
            if (VAL_CYCLE_DELAY==0) {
                VAL_STATE = STATE_WAIT_OFF;
            }
            break;
    }

    VAL_RUN_COUNTER++;

    // tws_delay(10); // delay 10ms
}