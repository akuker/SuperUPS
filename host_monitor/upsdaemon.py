#!/usr/bin/python3
# ---------------------------------------------------------------------------
#
# 	Raspberry Pi SuperUPS
#
# 	Copyright (C) 2020 Scott Baker
# 	Copyright (C) 2022 akuker
#
# 	[ Controller interface for the SuperUPS ]
#
# ---------------------------------------------------------------------------

import argparse
import daemon
import os
import signal
import sys
import time
import traceback
import logging

from ups import SuperUPS


SHUTDOWN_COMMAND = "/sbin/shutdown -h now",

# Global flags to terminate the daemon
glo_Usr1Received = False
glo_HupReceived = False

global this_ups

# create logger
global logger


def signal_hup(sig, stack):
    global glo_HupReceived
    glo_HupReceived = True


def signal_usr1(sig, stac):
    global glo_Usr1Received
    glo_Usr1Received = True


def monitor():
    global glo_Usr1Received, glo_HupReceived
    global this_ups, SHUTDOWN_COMMAND, logger

    debug_led_state = False
    # Counter to only check the voltage every second.
    count = 0

    signal.signal(signal.SIGHUP, signal_hup)
    signal.signal(signal.SIGUSR1, signal_usr1)
    logger.info("Starting monitor...")
    while True:
        try:
            count = count + 1
            if count > 999:
                count = 0

            v_in = this_ups.input_voltage
            v_threshold = this_ups.input_power_off_threshold
            logger.debug("VIN: %0.2f V_Threshold: %0.2f" % (v_in, v_threshold))
            logger.debug(this_ups.current_state_string)

            if(v_in < v_threshold):
                if (count % 10) == 0:
                    logger.warning("Power removed! VIN %0.2f is below threshold %0.2f !!!! Shutting down!!!" % (v_in, v_threshold))

            if(SuperUPS.States(this_ups.current_state) != SuperUPS.States.STATE_RUNNING):
                logger.warning("State has transitioned to " + this_ups.current_state_string)
                logger.warning("Commanding a Shutdown")
                os.system(SHUTDOWN_COMMAND)

            if glo_Usr1Received:
                glo_Usr1Received = False
                this_ups.dump_status()

            if glo_HupReceived:
                glo_HupReceived = False
                logger.warning("HUP Received!")
                logger.warning("exiting")
                return
                # logger.warning("Executing shutdown command %s" % SHUTDOWN_COMMAND)
                # os.system(SHUTDOWN_COMMAND)

            if (count % 10) == 0:
                # Toggle LED to show that this service is running
                logger.info("[Input Voltage: %0.2f] Setting LED to %d" % (v_in, debug_led_state))
                this_ups.debug_led_state = int(debug_led_state)
                debug_led_state = not debug_led_state

            # if debug logging is enabled, we'll pull the character buffer
            # from the UPS controller
            if logger.isEnabledFor(logging.DEBUG):
                while True:
                    debug_str = this_ups.read_string()
                    if(debug_str is None):
                        break
                    logger.debug("[ Controller message ] {}".format(debug_str))

        except:
            traceback.print_exc()

        time.sleep(0.1)


def sleep_forever():
    global logger
    logger.info("There doesn't seem to be a functional UPS attached. Going to sleep forever....")
    while not glo_HupReceived:
        logger.debug("Sleeping....")
        time.sleep(1)


if __name__ == "__main__":
    global my_ups
    global logger
    
    logger = logging.getLogger('upsdaemon')
    logger.setLevel(logging.INFO)

    # create console handler and set level to debug
    ch = logging.StreamHandler()

    # create formatter
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # add formatter to ch
    ch.setFormatter(formatter)
    # ch.setLevel(logging.DEBUG)

    # add ch to logger
    logger.addHandler(ch)

    try:
        this_ups = SuperUPS()
    except BaseException as err:
        logger.warning("An exception occurred while trying to create the UPS connection" + str(err))
        this_ups = None

    if this_ups is None or this_ups.is_invalid():
        sleep_forever()

    # context = daemon.DaemonContext(stdout=sys.stdout, stderr=sys.stderr)
    # with context:
    monitor()
