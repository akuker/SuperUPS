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

this_ups = SuperUPS()

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

    signal.signal(signal.SIGHUP, signal_hup)
    signal.signal(signal.SIGUSR1, signal_usr1)
    logger.info("Starting monitor...")
    while True:
        try:
            v_in = this_ups.input_voltage
            v_threshold = this_ups.input_power_off_threshold
            logger.info("VIN: %0.2f V_Threshold: %0.2f" % (v_in, v_threshold))

            if (v_in < v_threshold):
                logger.warning("VIN of %0.2f is less than threshold of %0.2f" % (v_in, v_threshold))
                logger.warning("Executing shutdown command %s" % SHUTDOWN_COMMAND)
                #os.system(SHUTDOWN_COMMAND)

            if glo_Usr1Received:
                glo_Usr1Received = False
                this_ups.dump_status()
                
            if glo_HupReceived:
                glo_HupReceived = False
                logger.warning("HUP Received!")
                logger.warning("Executing shutdown command %s" % SHUTDOWN_COMMAND)
                #os.system(SHUTDOWN_COMMAND)

            logger.debug("Setting LED to {}".format(debug_led_state))
            this_ups.debug_led_state = int(debug_led_state)
            debug_led_state = not debug_led_state
        except:
            traceback.print_exc()

        time.sleep(1)


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
    ch.setLevel(logging.DEBUG)

    # create formatter
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # add formatter to ch
    ch.setFormatter(formatter)

    # add ch to logger
    logger.addHandler(ch)

    if(not this_ups.is_valid()):
        sleep_forever()

    # context = daemon.DaemonContext(stdout=sys.stdout, stderr=sys.stderr)
    # with context:
    monitor()
