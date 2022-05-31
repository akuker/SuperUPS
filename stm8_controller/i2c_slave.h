#pragma once
#include "stm8s.h"


/********************** EXTERNAL FUNCTION **********************************/  
	void transaction_begin(void);
	void transaction_end(void);
	void byte_received(u8 u8_RxData);
	u8 byte_write(void);
	void Init_I2C(void);
	
#ifdef _RAISONANCE_
    void I2C_Slave_check_event(void) interrupt 19;
#elseif _COSMIC_
    @far @interrupt void I2C_Slave_check_event(void);
#else
    void I2C_Slave_check_event(void) __interrupt(19);
#endif

	
/********************** I2C configuration variables *****************************/  
	/* Define I2C Address mode ---------------------------------------------------*/
	#define I2C_slave_7Bits_Address

	/* Define Slave Address  -----------------------------------------------------*/
	#ifdef I2C_slave_10Bits_Address
		#define SLAVE_ADDRESS 0x3F0
	#endif 

	#ifdef I2C_slave_7Bits_Address
		#define SLAVE_ADDRESS 0x51
	#endif

	
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/