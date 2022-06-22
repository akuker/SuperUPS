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


class SuperUPS:

    class States(int):
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
    EXPECTED_INTERFACE_VERSION = 1

    def __init__(self, i2c_address=0x51):
        self.i2c_address = i2c_address
        self.bus = smbus.SMBus(1)
        if(self.interface_version != self.EXPECTED_INTERFACE_VERSION):
            print("WARNING: Unexpected interface version. SuperUPS may not work properly")
        self.cached_test_mode = False

    def __del__(self):
        self.bus.close()

    @property
    def input_voltage(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_ADC_VOLTAGE_IN)

    @input_voltage.setter
    def input_voltage(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_ADC_VOLTAGE_IN, new_value)

    @property
    def super_cap_voltage(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_ADC_SUPER_CAP_VOLTAGE)

    @super_cap_voltage.setter
    def super_cap_voltage(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_ADC_SUPER_CAP_VOLTAGE, new_value)

    @property
    def output_voltage(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_ADC_VOLTAGE_OUT)

    @output_voltage.setter
    def output_voltage(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_ADC_VOLTAGE_OUT, new_value)

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

    @property
    def input_power_on_threshold(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_12V_ON_THRESH)

    @input_power_on_threshold.setter
    def input_power_on_threshold(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_12V_ON_THRESH, new_value)

    @property
    def input_power_off_threshold(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_12V_OFF_THRESH)

    @input_power_off_threshold.setter
    def input_power_off_threshold(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_12V_OFF_THRESH, new_value)

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
    def ups_powerup_voltage_threshold(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_UPS_POWERUP_THRESH)

    @ups_powerup_voltage_threshold.setter
    def ups_powerup_voltage_threshold(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_UPS_POWERUP_THRESH, new_value)

    @property
    def shutdown_voltage_threshold(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_SHUTDOWN_THRESH)

    @shutdown_voltage_threshold.setter
    def shutdown_voltage_threshold(self, new_value):
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_SHUTDOWN_THRESH, new_value)

    @property
    def r1_value(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R1_VALUE)

    @r1_value.setter
    def r1_value(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(
            self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R1_VALUE, new_value)

    @property
    def r2_value(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R2_VALUE)

    @r2_value.setter
    def r2_value(self, new_value):
        assert self.cached_test_mode, str(inspect.currentframe().f_code.co_name) + \
            " can only be modified in test mode"
        self.bus.write_byte_data(self.i2c_address, self.I2C_VOLTAGE_DIVIDER_R2_VALUE, new_value)

    @property
    def interface_version(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_INTERFACE_VERSION)

    @property
    def build_id(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_BUILD_VERSION)

    @property
    def device_id(self):
        return self.bus.read_byte_data(self.i2c_address, self.I2C_DEVICE_ID)


if __name__ == "__main__":
    my_ups = SuperUPS()
    print("Device ID " + str(my_ups.device_id))
    my_ups.test_mode = True
    print("Test Mode: " + str(my_ups.test_mode))
    my_ups.input_voltage = 5
    print("Input Voltage: " + str(my_ups.input_voltage))
    my_ups.test_mode = False
    print("Test Mode: " + str(my_ups.test_mode))
    print("the following statement should crash")
    my_ups.input_voltage = 5
