//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ ADC Utilities ]
//
//---------------------------------------------------------------------------
#include <stdint.h>
#include <stm8s_adc1.h>
#include "adc_utilities.h"
#include "i2c_register_data.h"

#define MAX_ADC_SAMPLES 8

#define R1 100
#define R2 200

#define VCONV(x) (x * R2 / (R1 + R2))
#define VCONV_DIG(x) VCONV(x) * 256 / 2.56

#define GET_ON_THRESH uint8_t(VCONV_DIG(ON_THRESH_DESIRED))
#define GET_POWERUP_THRESH uint8_t(VCONV_DIG(POWERUP_THRESH_DESIRED))
#define GET_OFF_THRESH uint8_t(VCONV_DIG(OFF_THRESH_DESIRED))
#define GET_SHUTDOWN_THRESH uint8_t(VCONV_DIG(SHUTDOWN_THRESH_DESIRED))

static uint8_t sample_ptr = 0;
static uint8_t num_samples = 0;
static uint8_t vin_samples[MAX_ADC_SAMPLES];
static uint8_t vups_samples[MAX_ADC_SAMPLES];

static uint8_t computeAverage(uint8_t *samples)
{
    uint16_t accum = 0;
    uint8_t i;
    for (i = 0; i < num_samples; i++)
    {
        accum = accum + samples[i];
    }
    return accum / num_samples;
}

//---------------------------------------------------------------------------
// Based upon:
// https://circuitdigest.com/microcontroller-projects/adc-on-stm8s-using-c-compiler-reading-multiple-adc-values-and-displaying-on-lcd
//---------------------------------------------------------------------------
static uint8_t read_adc(ADC1_Channel_TypeDef ADC_Channel_Number)
{
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
    while (ADC1_GetFlagStatus(ADC1_FLAG_EOC) == FALSE)
        ;
    uint16_t result = ADC1_GetConversionValue();
    ADC1_ClearFlag(ADC1_FLAG_EOC);
    ADC1_DeInit();

    // The ADC will give us a 10 bit result. We want to
    // shift out the lowest 2 bits to generate a uint8
    return (uint8_t)((result >> 2) & 0xFF);
}

void adc_init()
{
    i2c_register_values[VIN_HIGH] = 0;
    i2c_register_values[VUPS_HIGH] = 0;
    i2c_register_values[R1_VALUE] = R1;
    i2c_register_values[R2_VALUE] = R2;
    // Enable Peripheral Clock for ADC
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, ENABLE);
}

void adc_step()
{
    vin_samples[sample_ptr] = read_adc(ADC1_CHANNEL_4);
    vups_samples[sample_ptr] = read_adc(ADC1_CHANNEL_3);
    sample_ptr = (sample_ptr + 1) % MAX_ADC_SAMPLES;
    num_samples++;
    if (num_samples > MAX_ADC_SAMPLES)
    {
        num_samples = MAX_ADC_SAMPLES;
    }

    i2c_register_values[VIN_HIGH] = computeAverage(vin_samples);
    i2c_register_values[VUPS_HIGH] = computeAverage(vups_samples);
}