#include "i2c_slave.h"
#define MAX_BUFFER  32

   u8 u8_My_Buffer[MAX_BUFFER];
   u8 *u8_MyBuffp;
   u8 MessageBegin;

// ********************** Data link function ****************************
// * These functions must be modified according to your application neeeds *
// * See AN document for more precision
// **********************************************************************

	void I2C_transaction_begin(void)
	{
		MessageBegin = TRUE;
	}
	void I2C_transaction_end(void)
	{
		//Not used in this example
	}
	void I2C_byte_received(u8 u8_RxData)
	{
		if (MessageBegin == TRUE  &&  u8_RxData < MAX_BUFFER) {
			u8_MyBuffp= &u8_My_Buffer[u8_RxData];
			MessageBegin = FALSE;
		}
    else if(u8_MyBuffp < &u8_My_Buffer[MAX_BUFFER])
      *(u8_MyBuffp++) = u8_RxData;
	}
	u8 I2C_byte_write(void)
	{
		if (u8_MyBuffp < &u8_My_Buffer[MAX_BUFFER])
			return *(u8_MyBuffp++);
		else
			return 0x00;
	}


// ********************** Data link interrupt handler *******************

void I2C_Slave_check_event(void) __interrupt(19) {

	static u8 sr1;					
	static u8 sr2;
	static u8 sr3;
	
// save the I2C registers configuration
sr1 = I2C->SR1;
sr2 = I2C->SR2;
sr3 = I2C->SR3;

/* Communication error? */
  if (sr2 & (I2C_SR2_WUFH | I2C_SR2_OVR |I2C_SR2_ARLO |I2C_SR2_BERR))
  {		
    I2C->CR2|= I2C_CR2_STOP;  // stop communication - release the lines
    I2C->SR2= 0;					    // clear all error flags
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
    I2C->SR2 &= ~I2C_SR2_AF;	  // clear AF
		I2C_transaction_end();
	}
/* Stop bit from Master  (= end of slave receive comm) */
  if (sr1 & I2C_SR1_STOPF) 
  {
    I2C->CR2 |= I2C_CR2_ACK;	  // CR2 write to clear STOPF
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
	GPIOD->ODR^=1;
}


// ************************* I2C init Function  *************************

void Init_I2C (void)
{
	#ifdef I2C_slave_7Bits_Address
		/* Set I2C registers for 7Bits Address */
		I2C->CR1 |= 0x01;				        	// Enable I2C peripheral
		I2C->CR2 = 0x04;					      		// Enable I2C acknowledgement
		I2C->FREQR = 16; 					      	// Set I2C Freq value (16MHz)
		I2C->OARL = (SLAVE_ADDRESS << 1) ;	// set slave address to 0x51 (put 0xA2 for the register dues to7bit address) 
		I2C->OARH = 0x40;					      	// Set 7bit address mode

	#endif
	#ifdef I2C_slave_10Bits_Address
	  /* Set I2C registers for 10Bits Address */
	  I2C->CR1 |= 0x01;				  // Enable I2C peripheral
	  I2C->CR2 = 0x04;					// Enable I2C acknowledgement
	  I2C->FREQR = 16; 					// Set I2C Freq value (16MHz)
	  I2C->OARL = (SLAVE_ADDRESS & 0xFF) ;							// set slave address LSB 
	  I2C->OARH = 0xC0 | ((SLAVE_ADDRESS & 0x300) >> 7);	// Set 10bits address mode and address MSB
	#endif
	
	I2C->ITR	= 0x07;					      // all I2C interrupt enable  
}
