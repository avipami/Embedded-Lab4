/*By Vinnie!*/
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "serial.h"
#include "timer.h"

void main (void) 
{
	uint8_t eeprom_output_byte = 0; 
	i2c_init();
	uart_init();
	sei();
	eeprom_write_byte(EEPROM_MEM_ADDR,'V'); // V for Vincent!
//Deluppg2
	eeprom_write_byte(EEPROM_MEM_ADDR + 1,'I'); // V for Vincent!
	eeprom_write_byte(EEPROM_MEM_ADDR + 2,'N'); // V for Vincent!
	eeprom_write_byte(EEPROM_MEM_ADDR + 3,'C'); // V for Vincent!
	eeprom_write_byte(EEPROM_MEM_ADDR + 4,'E'); // V for Vincent!
	eeprom_write_byte(EEPROM_MEM_ADDR + 5,'N'); // V for Vincent!
	eeprom_write_byte(EEPROM_MEM_ADDR + 6,'T'); // V for Vincent!
 
	while (1) 
	{
		eeprom_output_byte = eeprom_read_byte(EEPROM_MEM_ADDR + eeprom_output_byte/*deluppg 2*/);
//Deluppg2
		eeprom_output_byte ++;
		if (eeprom_output_byte == 7)
		{
			eeprom_output_byte = 0;
			printf_P(PSTR ("%c\n"));
		}
		
		printf_P(PSTR ("%c\n"),eeprom_output_byte);
	}
}

