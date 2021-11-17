#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "i2c.h"

void i2c_init(void) {
	// SCL frequency = CPU/ 16 + 2*(TWBR) * PrescalerValue
	/* TWBR = ((16 000 000 / 100 000) -16) / 2 = 72  0x48
	 * 
	 */
	TWBR = 0x48;
	TWSR = 0x00;
	TWCR |= _BV(TWEN);

}

void i2c_meaningful_status(uint8_t status) {
	switch (status) {
		case 0x08: // START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("START\n"));
			break;
		case 0x10: // repeated START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("RESTART\n"));
			break;
		case 0x38: // NAK or DATA ARBITRATION LOST
			printf_P(PSTR("NOARB/NAK\n"));
			break;
		// MASTER TRANSMIT
		case 0x18: // SLA+W transmitted, ACK received
			printf_P(PSTR("MT SLA+W, ACK\n"));
			break;
		case 0x20: // SLA+W transmitted, NAK received
			printf_P(PSTR("MT SLA+W, NAK\n"));
				break;
		case 0x28: // DATA transmitted, ACK received
			printf_P(PSTR("MT DATA+W, ACK\n"));
			break;
		case 0x30: // DATA transmitted, NAK received
			printf_P(PSTR("MT DATA+W, NAK\n"));
			break;
		// MASTER RECEIVE
		case 0x40: // SLA+R transmitted, ACK received
			printf_P(PSTR("MR SLA+R, ACK\n"));
			break;
		case 0x48: // SLA+R transmitted, NAK received
			printf_P(PSTR("MR SLA+R, NAK\n"));
			break;
		case 0x50: // DATA received, ACK sent
			printf_P(PSTR("MR DATA+R, ACK\n"));
			break;
		case 0x58: // DATA received, NAK sent
			printf_P(PSTR("MR DATA+R, NAK\n"));
			break;
		default:
			printf_P(PSTR("N/A %02X\n"), status);
			break;
	}
}

inline void i2c_start() {
	// ...
	/*
	Bit 2 – TWEN: TWI Enable
	Bit 5 – TWSTA: TWI START Condition
	Bit 7 – TWINT: TWI Interrupt Flag
	This bit is set by hardware when the TWI has finished its current job

	*/
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1 << TWINT)));

}

inline void i2c_stop() {
	// ...
	/*
	Bit 4 – TWSTO: TWI STOP Condition
	Writing the TWSTO bit to one in Master mode will generate a STOP condition
	*/
	TWCR = (1 << TWINT) | (1 << TWEN) | (1<<TWSTO);
	while ((TWCR & (1 << TWSTO)));
}

inline uint8_t i2c_get_status(void) {
	// ... Check i2c bus Bits 3, 4, 5, 6, 7 – TWSn: TWI Status Bit   Reset:  0xF8
	//
	//Bits 0, 1 – TWPSn: TWI Prescaler
	//These bits can be read and written, and control the bit rate prescaler.
	return TWSR & 0xF8;  
}

inline void i2c_xmit_addr(uint8_t address, uint8_t rw) 
{// ...
	TWDR = (address & 0xFE) | (rw & 0x01); // TWDR This unit contains the Data and Address Shift Register

	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1<<TWINT))); // when finished TWINT will turn into false and stop data rw
	/*
	This bit is set by hardware when the TWI has finished its current job 
	and expects application software response. 
	If the I-bit in SREG and TWIE in TWCR are set, the MCU will jump to the TWI Interrupt Vector. 
	While the TWINT Flag is set, the SCL low period is stretched. 
	The TWINT Flag must be cleared by software by writing a logic one to it.
	*/

}

inline void i2c_xmit_byte(uint8_t data) 
{// ...
	TWDR = data;// TWDR This unit contains the Data and Address Shift Register

	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1<<TWINT)));
}

inline uint8_t i2c_read_ACK() 
{// ...
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1<<TWINT)));
	return TWDR; // TWDR This unit contains the Data and Address Shift Register contains ACK
}

inline uint8_t i2c_read_NAK()  // NOT ACK :P
{// ...
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;// TWDR This unit contains the Data and Address Shift Register  contains NACK
}

inline void eeprom_wait_until_write_complete() 
{
	while (i2c_get_status() != 0x18) //0x18 Checks if ACK or NACK is transmitted, ACK stops the write data, 
									 //NACK keeps the data transfer active
	{
		i2c_start();
		i2c_xmit_addr(EEPROM_ADDR,I2C_W);
	}
}

uint8_t eeprom_read_byte(uint8_t addr)  // addr MEM ADDR
{
	// ...
	uint8_t readDataByte;
	i2c_start();
	i2c_xmit_addr(EEPROM_ADDR,I2C_W); // Connect Write
	i2c_xmit_byte(addr); // Send to MEM ADDR
	//Restart
	i2c_start();
	i2c_xmit_addr(EEPROM_ADDR,I2C_R); // Connect Read
	readDataByte = i2c_read_NAK();// Store data
	i2c_stop(); // stop,, duuh
	return readDataByte;
}

void eeprom_write_byte(uint8_t addr, uint8_t data) {
	// ...
	i2c_start();
	i2c_xmit_addr(EEPROM_ADDR,I2C_W); // Connect Write
	i2c_xmit_byte(addr); // Send to MEM ADDR

	i2c_xmit_byte(data); // Connect Read
	i2c_stop();
	eeprom_wait_until_write_complete();

}



void eeprom_write_page(uint8_t addr, uint8_t *data) {
	// ... (VG)
}

void eeprom_sequential_read(uint8_t *buf, uint8_t start_addr, uint8_t len) {
	// ... (VG)
}
