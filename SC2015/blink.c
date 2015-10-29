#define F_CPU 8000000  // CPU frequency for proper time calculation in delay function

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	DDRB |= (1 << PB0);  // make PD6 an output

	for(;;)
	{
		PORTB ^= (1 << PB0);  // toggle PD6
		_delay_ms(1000);  // delay for a second
	}

	return 0;  // the program executed successfully
}