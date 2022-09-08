#!/usr/bin/python3
# ---------------------------------------------------------------------------
#
# 	Raspberry Pi SuperUPS
#
# 	Copyright (C) 2020 Scott Baker
# 	Copyright (C) 2022 akuker
#
# 	[ Python class for reading/writting to the UPS controller ]
#
# ---------------------------------------------------------------------------

import smbus
import inspect
import logging
import enum
import ups



if __name__ == "__main__":
    global my_ups
    global logger
    
    logger = logging.getLogger('upsdaemon')
    logger.setLevel(logging.INFO)

    # create console handler and set level to debug
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)

    # create formatter
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # add formatter to ch
    ch.setFormatter(formatter)

    # add ch to logger
    logger.addHandler(ch)
    my_ups = ups.SuperUPS()
    print("Device ID " + str(my_ups.device_id))
    prev_reading = 0
    prev_reading2 = 0
    prev_reading3 = 0

    # while True:
    #     v_in = my_ups.input_voltage
    #     v_threshold = my_ups.input_power_off_threshold
    #     logger.info("VIN: %0.2f V_Threshold: %0.2f" % (v_in, v_threshold))
    #     logger.info(my_ups.current_state_string)

    while True:
        new_reading = my_ups.bus.read_byte_data(my_ups.i2c_address, my_ups.I2C_ADC_VOLTAGE_IN)
        if (new_reading > (prev_reading + 2)) or (new_reading < (prev_reading - 2)):
            print("adc voltage: " + str(hex(new_reading)))
            prev_reading = new_reading
        my_ups.debug_led_state = 1
        my_ups.test_mode = 1
        my_ups.read_string()

        new_reading2 = my_ups.bus.read_byte_data(my_ups.i2c_address, my_ups.I2C_12V_OFF_THRESH)
        if (new_reading2 > prev_reading2) or (new_reading2 < prev_reading2):
            print("off threshold: " + str(hex(new_reading2)))
            prev_reading2 = new_reading2
        my_ups.debug_led_state = 0
        my_ups.test_mode = 0

        new_reading3 = my_ups.bus.read_byte_data(my_ups.i2c_address, my_ups.I2C_ADC_VOLTAGE_IN)
        new_reading3 = my_ups._convert_raw_to_voltage(new_reading3)
        if (new_reading3 > (prev_reading3 + 2)) or (new_reading3 < (prev_reading3 - 2)):
            print("adc voltage: " + str(new_reading3))
            prev_reading3 = new_reading3
        my_ups.debug_led_state = 1
        my_ups.test_mode = 1
        my_ups.read_string()

    my_ups.dump_status()
    my_ups.test_mode = True
    print("Test Mode: " + str(my_ups.test_mode))
    my_ups.input_voltage = 5
    print("Input Voltage: " + str(my_ups.input_voltage))
    my_ups.test_mode = False
    print("Test Mode: " + str(my_ups.test_mode))
    print("the following statement should crash")
    my_ups.input_voltage = 5
