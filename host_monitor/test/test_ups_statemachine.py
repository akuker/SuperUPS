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

import unittest
import ups
import datetime

class TestUpsSettersAndGetters(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.ups = ups.SuperUPS()
        # Save off the original configuration settings, so we can put them back later
        cls.original_input_power_on_threshold = cls.ups.input_power_on_threshold
        cls.original_input_power_off_threshold = cls.ups.input_power_off_threshold
        cls.original_ups_powerup_voltage_threshold = cls.ups.ups_powerup_voltage_threshold
        cls.original_shutdown_voltage_threshold = cls.ups.shutdown_voltage_threshold
        cls.original_r1_value = cls.ups.r1_value
        cls.original_r2_value = cls.ups.r2_value

    @classmethod
    def tearDownClass(cls):
        # Restore original values, in case they were changed
        cls.ups.test_mode = True
        cls.ups.input_power_on_threshold = cls.original_input_power_on_threshold
        cls.ups.input_power_off_threshold = cls.original_input_power_off_threshold
        cls.ups.ups_powerup_voltage_threshold = cls.original_ups_powerup_voltage_threshold
        cls.ups.shutdown_voltage_threshold = cls.original_shutdown_voltage_threshold
        cls.ups.r1_value = cls.original_r1_value
        cls.ups.r2_value = cls.original_r2_value
        cls.ups.test_mode = False
        del cls.ups

    def wait_for_equal_timeout(a, b, timeout_seconds):
        """
        Wait for a to equal b, but not more than <timeout> seconds
        """
        start = datetime.datetime.now()
        while(a != b):
            time_delta = datetime.datetime.now - start
            if time_delta.total_seconds() > timeout_seconds:
                break

    def test_commanded_power_cycle(self):
        """
        When a power cycle is commanded, the mosfet should be disabled
        for a few seconds, then re-enabled
        """
        self.ups.test_mode = True
        self.assertEqual(self.ups.current_state, ups.SuperUPS.States.STATE_RUNNING)
        print("Setting to true")
        # Shutdown counter is specified in 10s of mm
        self.ups.command_shutdown_counter = 100
        with self.assertRaises(AssertionError):
            self.ups.current_state = ups.SuperUPS.States.STATE_DISABLED

    def test_input_voltage_out_of_test_mode(self):
        """
        Input voltage can NOT be overwritten when out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AssertionError):
            self.ups.input_voltage = 0

    def test_output_voltage_out_of_test_mode(self):
        """
        Output voltage can NOT be overwritten when out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AssertionError):
            self.ups.output_voltage = 0

    def test_super_cap_voltage_out_of_test_mode(self):
        """
        Super cap voltage can NOT be overwritten when out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AssertionError):
            self.ups.super_cap_voltage = 0

    def test_r1_out_of_test_mode(self):
        """
        R1 variable can NOT be overwritten when out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AssertionError):
            self.ups.r1_value = 0

    def test_r2_out_of_test_mode(self):
        """
        R1 variable can NOT be overwritten when out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AssertionError):
            self.ups.r2_value = 0

    def test_device_id_is_read_only(self):
        """
        Device ID can not be written in or out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AttributeError):
            self.ups.device_id = 0
        self.ups.test_mode = True
        with self.assertRaises(AttributeError):
            self.ups.device_id = 0

    def test_build_id_is_read_only(self):
        """
        Build ID can not be written in or out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AttributeError):
            self.ups.build_id = 0
        self.ups.test_mode = True
        with self.assertRaises(AttributeError):
            self.ups.build_id = 0

    def test_interface_version_is_read_only(self):
        """
        Interface Version can not be written in or out of test mode
        """
        self.ups.test_mode = False
        with self.assertRaises(AttributeError):
            self.ups.interface_version = 0
        self.ups.test_mode = True
        with self.assertRaises(AttributeError):
            self.ups.interface_version = 0

if __name__ == '__main__':
    log_file = 'log_file.txt'
    with open(log_file, "w") as f:
        runner = unittest.TextTestRunner(f)
        unittest.main(testRunner=runner)
