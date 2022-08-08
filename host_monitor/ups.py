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


class SuperUPS:

    class States(enum.Enum):
        STATE_DISABLED = 0
        """Invalid state - not used by the controller."""
        STATE_WAIT_OFF = 1
        """During a commanded power cycle, delay in the 'off' state"""
        STATE_WAIT_ON = 2
        """Turn the power back on after a power cycle"""
        STATE_POWERUP = 3
        """Waiting for the super caps to charge after input power is restored"""
        STATE_RUNNING = 4
        """Normal running state of the super UPS"""
        STATE_POWER_LOSS_SHUTDOWN = 5
        """Wait for input power to be restored"""
        STATE_POWER_LOSS_SHUTDOWN_DELAY = 6
        """Input power is lost and the super caps have been discharged."""
        STATE_COMMANDED_POWER_CYCLE_DELAY = 7
        """Pi has commanded a power cycle. Remove power and delay"""

    I2C_CHARACTER_BUFFER = 1
    I2C_TEST_MODE_ENABLE = 2
    I2C_ADC_VOLTAGE_IN = 3
    I2C_ADC_VOLTAGE_OUT = 4
    I2C_ADC_SUPER_CAP_VOLTAGE = 5
    I2C_MOSFET_ENABLE = 6
    I2C_12V_ON_THRESH = 7
    I2C_12V_OFF_THRESH = 8
    I2C_COUNTDOWN = 9
    I2C_CURENT_STATE = 10
    I2C_CYCLE_DELAY = 11
    I2C_FAIL_SHUTDOWN_DELAY = 12
    I2C_RUN_COUNTER = 13
    I2C_UPS_POWERUP_THRESH = 14
    I2C_SHUTDOWN_THRESH = 15
    I2C_VOLTAGE_DIVIDER_R1_VALUE = 16
    I2C_VOLTAGE_DIVIDER_R2_VALUE = 17
    I2C_INTERFACE_VERSION = 18
    I2C_BUILD_VERSION = 19
    I2C_DEVICE_ID = 20
    I2C_POWER_BUTTON_STATE = 21
    I2C_AUX_BUTTON_STATE = 22
    I2C_DEBUG_LED_STATE = 23

    EXPECTED_INTERFACE_VERSION = 1

    def __init__(self, i2c_address=0x52):
        self.i2c_address = i2c_address
        self.logger = logging.getLogger('upsdaemon')
        try:
            self.bus = smbus.SMBus(1)
            if(self.interface_version != self.EXPECTED_INTERFACE_VERSION):
                print("WARNING: Unexpected interface version. SuperUPS may not work properly")
            self.cached_test_mode = False
            self.string_cache = ""
        except BaseException as err:
            self.logger.warning("Unable to connect to UPS controller: " + str(err))
            self.bus = None
            self.cached_test_mode = False

    def __del__(self):
        self.bus.close()

    #region debug text handling

    def _read_character(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_CHARACTER_BUFFER)

    def read_string(self):
        logger = logging.getLogger('upsdaemon')
        while True:
            next_character = self._read_character()
            if(next_character == 0xFF):
                break
            else:
                if(chr(next_character) == '\n'):
                    return_string = self.string_cache
                    self.string_cache = ""
                    return return_string
                elif(chr(next_character) == '\r'):
                    pass
                elif(chr(next_character) == '\0'):
                    pass
                else:
                    logger.debug("Read char: {} (0x{})".format(chr(next_character), hex(next_character)))
                    self.string_cache = self.string_cache + chr(next_character)

        return None
    #endregion

    #region Voltages
    @property
    def input_voltage_raw(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_ADC_VOLTAGE_IN)

    @input_voltage_raw.setter
    def input_voltage_raw(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_ADC_VOLTAGE_IN, new_value)

    @property
    def input_voltage(self):
        return self._convert_raw_to_voltage(self.input_voltage_raw)

    @input_voltage.setter
    def input_voltage(self, new_value):
        self.input_voltage_raw = self._convert_voltage_to_raw(new_value)

    @property
    def super_cap_voltage_raw(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_ADC_SUPER_CAP_VOLTAGE)

    @super_cap_voltage_raw.setter
    def super_cap_voltage_raw(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_ADC_SUPER_CAP_VOLTAGE, new_value)

    @property
    def super_cap_voltage(self):
        return self._convert_raw_to_voltage(self.super_cap_voltage_raw)

    @super_cap_voltage.setter
    def super_cap_voltage(self, new_value):
        self.super_cap_voltage_raw = self._convert_voltage_to_raw(new_value)

    @property
    def output_voltage_raw(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_ADC_VOLTAGE_OUT)

    @output_voltage_raw.setter
    def output_voltage_raw(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_ADC_VOLTAGE_OUT, new_value)

    @property
    def output_voltage(self):
        return self._convert_raw_to_voltage(self.output_voltage_raw)

    @output_voltage.setter
    def output_voltage(self, new_value):
        self.output_voltage_raw = self._convert_voltage_to_raw(new_value)

    def _convert_raw_to_voltage(self, raw_adc_value):
        r1 = self.r1_value
        r2 = self.r2_value
        # voltage into adc..... (raw_voltage/256) * 3.3
        # voltage divder formula.... Vout = (Vin * R2)/(R1+R2)
        # therefore Vin = (R1+R2) * (Vout/R2)
        voltage_out = (raw_adc_value/256) * 3.3
        voltage_in = (r1 + r2) * (voltage_out / r2)
        return_path = self._convert_voltage_to_raw(voltage_in)
        self.logger.debug("r1: {r1} r2: {r2} raw_in:{raw_input} adc_v:{adc_v} actual_v:{output} test_value:{test_value}".format(r1=r1, r2=r2, raw_input=raw_adc_value, adc_v=voltage_out, output=voltage_in, test_value=return_path))
        return voltage_in

    def _convert_voltage_to_raw(self, voltage):
        r1 = self.r1_value
        r2 = self.r2_value
        raw_voltage = (voltage * r2)/(r1+r2) * (256/3.3)
        self.logger.debug("r1: {r1} r2: {r2} voltage:{voltage} raw_value:{raw_value}".format(r1=r1, r2=r2, voltage=voltage, raw_value=raw_voltage))
        return int(raw_voltage)
    
    #endregion

    @property
    def test_mode(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_TEST_MODE_ENABLE)

    @test_mode.setter
    def test_mode(self, new_value):
        self.cached_test_mode = bool(new_value)
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_TEST_MODE_ENABLE, new_value)

    @property
    def power_on(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_MOSFET_ENABLE)

    #region Power Thresholds

    @property
    def input_power_on_threshold_raw(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_12V_ON_THRESH)

    @input_power_on_threshold_raw.setter
    def input_power_on_threshold_raw(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_12V_ON_THRESH, new_value)

    @property
    def input_power_on_threshold(self):
        return self._convert_raw_to_voltage(self.input_power_on_threshold_raw)

    @input_power_on_threshold.setter
    def input_power_on_threshold(self, new_value):
        self.input_power_on_threshold_raw = self._convert_voltage_to_raw(new_value)

    @property
    def input_power_off_threshold_raw(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_12V_OFF_THRESH)

    @input_power_off_threshold_raw.setter
    def input_power_off_threshold_raw(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_12V_OFF_THRESH, new_value)

    @property
    def input_power_off_threshold(self):
        return self._convert_raw_to_voltage(self.input_power_off_threshold_raw)

    @input_power_off_threshold.setter
    def input_power_off_threshold(self, new_value):
        self.input_power_off_threshold_raw = self._convert_voltage_to_raw(new_value)

    @property
    def ups_powerup_voltage_threshold_raw(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_UPS_POWERUP_THRESH)

    @ups_powerup_voltage_threshold_raw.setter
    def ups_powerup_voltage_threshold_raw(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_UPS_POWERUP_THRESH, new_value)

    @property
    def ups_powerup_voltage_threshold(self):
        return self._convert_raw_to_voltage(self.ups_powerup_voltage_threshold_raw)

    @ups_powerup_voltage_threshold.setter
    def ups_powerup_voltage_threshold(self, new_value):
        self.ups_powerup_voltage_threshold_raw = self._convert_voltage_to_raw(new_value)

    @property
    def shutdown_voltage_threshold_raw(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_SHUTDOWN_THRESH)

    @shutdown_voltage_threshold_raw.setter
    def shutdown_voltage_threshold_raw(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_SHUTDOWN_THRESH, new_value)

    @property
    def shutdown_voltage_threshold(self):
        return self._convert_raw_to_voltage(self.shutdown_voltage_threshold_raw)

    @shutdown_voltage_threshold.setter
    def shutdown_voltage_threshold(self, new_value):
        self.shutdown_voltage_threshold_raw = self._convert_voltage_to_raw(new_value)
    #endregion 

    @property
    def current_state(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_CURENT_STATE)

    @current_state.setter
    def current_state(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_CURENT_STATE, new_value)

    @property
    def current_state_string(self):
        return str(SuperUPS.States(self.current_state))

    @property
    def r1_value(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R1_VALUE)

    @r1_value.setter
    def r1_value(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.cached_r1 = new_value
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R1_VALUE, new_value)

    @property
    def r2_value(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R2_VALUE)

    @r2_value.setter
    def r2_value(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.cached_r2 = new_value
        self.bus.write_byte_data(self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R2_VALUE, new_value)

    @property
    def command_shutdown_counter(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_COUNTDOWN)

    @command_shutdown_counter.setter
    def command_shutdown_counter(self, new_value):
        self.bus.write_byte_data(self.i2c_address, self.I2C_COUNTDOWN, new_value)

    @property
    def interface_version(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_INTERFACE_VERSION)

    @property
    def build_id(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_BUILD_VERSION)

    @property
    def device_id(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_DEVICE_ID)

    @property
    def power_button_state(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_POWER_BUTTON_STATE)

    @power_button_state.setter
    def power_button_state(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_POWER_BUTTON_STATE, new_value)

    @property
    def aux_button_state(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_AUX_BUTTON_STATE)

    @aux_button_state.setter
    def aux_button_state(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_AUX_BUTTON_STATE, new_value)

    @property
    def debug_led_state(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_DEBUG_LED_STATE)

    @debug_led_state.setter
    def debug_led_state(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_DEBUG_LED_STATE, new_value)

    def dump_status(self):
        print("({num:2}) I2C_TEST_MODE_ENABLE = {value}".format(num=self.I2C_TEST_MODE_ENABLE, value=self.test_mode))
        print("({num:2}) I2C_ADC_VOLTAGE_IN = {value}".format(num=self.I2C_ADC_VOLTAGE_IN, value=self.input_voltage))
        print("({num:2}) I2C_ADC_VOLTAGE_OUT = {value}".format(num=self.I2C_ADC_VOLTAGE_OUT, value=self.output_voltage))
        print("({num:2}) I2C_ADC_SUPER_CAP_VOLTAGE = {value}".format(num=self.I2C_ADC_SUPER_CAP_VOLTAGE, value=self.super_cap_voltage))
        print("({num:2}) I2C_MOSFET_ENABLE = {value}".format(num=self.I2C_MOSFET_ENABLE, value=self.power_on))
        print("({num:2}) I2C_12V_ON_THRESH = {value}".format(num=self.I2C_12V_ON_THRESH, value=self.input_power_on_threshold))
        print("({num:2}) I2C_12V_OFF_THRESH = {value}".format(num=self.I2C_12V_OFF_THRESH, value=self.input_power_off_threshold))
        print("({num:2}) I2C_COUNTDOWN = {value}".format(num=self.I2C_COUNTDOWN, value=self.command_shutdown_counter))
        print("({num:2}) I2C_CURENT_STATE = {value}".format(num=self.I2C_CURENT_STATE, value=self.current_state))
        print("({num:2}) I2C_CYCLE_DELAY = {value}".format(num=self.I2C_CYCLE_DELAY, value=self.bus.read_byte_data(self.i2c_address, self.I2C_CYCLE_DELAY)))
        print("({num:2}) I2C_FAIL_SHUTDOWN_DELAY = {value}".format(num=self.I2C_FAIL_SHUTDOWN_DELAY, value=self.bus.read_byte_data(self.i2c_address, self.I2C_FAIL_SHUTDOWN_DELAY)))
        print("({num:2}) I2C_RUN_COUNTER = {value}".format(num=self.I2C_RUN_COUNTER, value=self.bus.read_byte_data(self.i2c_address, self.I2C_RUN_COUNTER)))
        print("({num:2}) I2C_UPS_POWERUP_THRESH = {value}".format(num=self.I2C_UPS_POWERUP_THRESH, value=self.bus.read_byte_data(self.i2c_address, self.I2C_FAIL_SHUTDOWN_DELAY)))
        print("({num:2}) I2C_SHUTDOWN_THRESH = {value}".format(num=self.I2C_SHUTDOWN_THRESH, value=self.shutdown_voltage_threshold))
        print("({num:2}) I2C_VOLTAGE_DIVIDER_R1_VALUE = {value}".format(num=self.I2C_VOLTAGE_DIVIDER_R1_VALUE, value=self.r1_value))
        print("({num:2}) I2C_VOLTAGE_DIVIDER_R2_VALUE = {value}".format(num=self.I2C_VOLTAGE_DIVIDER_R2_VALUE, value=self.r2_value))
        # print(("{num}) I2C_INTERFACE_VERSION = {value}".format(num=self.I2C_INTERFACE_VERSION , )
        # print("{num} I2C_BUILD_VERSION = {value}".format(num=self.I2C_BUILD_VERSION , )
        # print("{num} I2C_DEVICE_ID = {value}".format(num=self.I2C_DEVICE_ID , )
        print("({num:2}) I2C_POWER_BUTTON_STATE = {value}".format(num=self.I2C_POWER_BUTTON_STATE, value=self.power_button_state))
        print("({num:2}) I2C_AUX_BUTTON_STATE = {value}".format(num=self.I2C_AUX_BUTTON_STATE, value=self.aux_button_state))
        print("({num:2}) I2C_DEBUG_LED_STATE = {value}".format(num=self.I2C_DEBUG_LED_STATE, value=self.debug_led_state))

    def is_valid(self):
        if(self.bus is None):
            return False
        if(self.interface_version != self.EXPECTED_INTERFACE_VERSION):
            return False
        return True

    def is_invalid(self):
        return not self.is_valid()


if __name__ == "__main__":
    my_ups = SuperUPS()
    print("Device ID " + str(my_ups.device_id))
    my_ups.dump_status()
    my_ups.test_mode = True
    print("Test Mode: " + str(my_ups.test_mode))
    my_ups.input_voltage = 5
    print("Input Voltage: " + str(my_ups.input_voltage))
    my_ups.test_mode = False
    print("Test Mode: " + str(my_ups.test_mode))
    print("the following statement should crash")
    my_ups.input_voltage = 5
