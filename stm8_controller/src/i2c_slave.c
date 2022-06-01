//---------------------------------------------------------------------------
//
//	Raspberry Pi SuperUPS
//
//  Copyright (C) 2009 STMicroelectronics
//	Copyright (C) 2020 Scott Baker
//	Copyright (C) 2022 akuker
//
//	[ I2C slave handler ]
//
//  The I2C slave handling is based upon STMicroelectronics AN3281
//     https://www.st.com/en/embedded-software/stsw-stm8004.html
//
//---------------------------------------------------------------------------
#include "i2c_slave.h"
#include "i2c_register_data.h"

u8 *u8_MyBuffp;
u8 MessageBegin;
volatile u8 i2c_counter = 0;

// void transaction_begin(void);
// void transaction_end(void);
// void byte_received(u8 u8_RxData);
// u8 byte_write(void);

static void I2C_transaction_begin(void)
{
	MessageBegin = TRUE;
	i2c_counter++;
}

static void I2C_transaction_end(void)
{
	// Not used in this example
}

static void I2C_byte_received(u8 u8_RxData)
{
	if (MessageBegin == TRUE && u8_RxData < REGISTERS_SIZE)
	{
		u8_MyBuffp = &i2c_register_values[u8_RxData];
		MessageBegin = FALSE;
	}
	else if (u8_MyBuffp <= &i2c_register_values[REGISTERS_SIZE - 1])
		*(u8_MyBuffp++) = u8_RxData;
}

static u8 I2C_byte_write(void)
{
	if (u8_MyBuffp <= &i2c_register_values[REGISTERS_SIZE - 1])
		return *(u8_MyBuffp++);
	else
		return 0x00;
}

void I2C_Slave_check_event(void) __interrupt(19)
{

	static u8 sr1;
	static u8 sr2;
	static u8 sr3;

	// save the I2C registers configuration
	sr1 = I2C->SR1;
	sr2 = I2C->SR2;
	sr3 = I2C->SR3;

	/* Communication error? */
	if (sr2 & (I2C_SR2_WUFH | I2C_SR2_OVR | I2C_SR2_ARLO | I2C_SR2_BERR))
	{
		I2C->CR2 |= I2C_CR2_STOP; // stop communication - release the lines
		I2C->SR2 = 0;			  // clear all error flags
	}
	/* More bytes received ? */
	if ((sr1 & (I2C_SR1_RXNE | I2C_SR1_BTF)) == (I2C_SR1_RXNE | I2C_SR1_BTF))
	{
		I2C_byte_received(I2C->DR);
	}
	/* Byte received ? */
	if (sr1 & I2C_SR1_RXNE)
	{
		I2C_byte_received(I2C->DR);
	}
	/* NAK? (=end of slave transmit comm) */
	if (sr2 & I2C_SR2_AF)
	{
		I2C->SR2 &= ~I2C_SR2_AF; // clear AF
		I2C_transaction_end();
	}
	/* Stop bit from Master  (= end of slave receive comm) */
	if (sr1 & I2C_SR1_STOPF)
	{
		I2C->CR2 |= I2C_CR2_ACK; // CR2 write to clear STOPF
		I2C_transaction_end();
	}
	/* Slave address matched (= Start Comm) */
	if (sr1 & I2C_SR1_ADDR)
	{
		I2C_transaction_begin();
	}
	/* More bytes to transmit ? */
	if ((sr1 & (I2C_SR1_TXE | I2C_SR1_BTF)) == (I2C_SR1_TXE | I2C_SR1_BTF))
	{
		I2C->DR = I2C_byte_write();
	}
	/* Byte to transmit ? */
	if (sr1 & I2C_SR1_TXE)
	{
		I2C->DR = I2C_byte_write();
	}
	GPIOD->ODR ^= 1;
}

void i2c_init(void)
{
	/* Init GPIO for I2C use */
	GPIOE->CR1 |= 0x06;
	GPIOE->DDR &= ~0x06;
	GPIOE->CR2 &= ~0x06;

	/* Set I2C registers for 7Bits Address */
	I2C->CR1 |= 0x01;						   // Enable I2C peripheral
	I2C->CR2 = 0x04;						   // Enable I2C acknowledgement
	I2C->FREQR = 16;						   // Set I2C Freq value (16MHz)
	I2C->OARL = (uint8_t)(SLAVE_ADDRESS << 1); // set slave address to 0x51 (put 0xA2 for the register dues to7bit address)
	I2C->OARH = 0x40;						   // Set 7bit address mode

	I2C->ITR = 0x07; // all I2C interrupt enable
}
