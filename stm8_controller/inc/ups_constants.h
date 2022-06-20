//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ Constants for SuperUPS processing ]
//
//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

typedef enum
{
    STATE_DISABLED = (uint8_t)0,
    STATE_WAIT_OFF = (uint8_t)1,
    STATE_WAIT_ON = (uint8_t)2,
    STATE_POWERUP = (uint8_t)3,
    STATE_RUNNING = (uint8_t)4,
    STATE_POWER_LOSS_SHUTDOWN = (uint8_t)5,
    STATE_POWER_LOSS_SHUTDOWN_DELAY = (uint8_t)6,
    STATE_COMMANDED_POWER_CYCLE_DELAY = (uint8_t)7,
} ups_state_type;
