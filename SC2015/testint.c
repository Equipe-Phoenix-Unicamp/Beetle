#define F_CPU 8000000

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


/***********************************************************************************************************************************/
//Here we use 32 bit precision so it will only overflow in more than 3 minutes (battle time).                                      //
volatile uint32_t timovf_count = 0;

volatile uint32_t ch0 = 0;
volatile uint32_t ch0_last = 0;

volatile uint32_t ch1 = 0;
volatile uint32_t ch1_last = 0;
/***********************************************************************************************************************************/
ISR(INT0_vect)
{
	PORTD ^= (1 << PD5);
}

ISR(INT1_vect)
{
	PORTD ^= (1 << PD6);
}

int main(void)
{
	CLKPR = 0; //Sets internal RC oscilator clock to 8MHz
	
	DDRD |= (1 << PD6) | (1 << PD5); //OC0A and OC0B
	DDRB |= (1 << PB2) | (1 << PB1); //OC1A and OC1B
	
	EICRA |= (1 << ISC10) | (1 << ISC00); //Configures interrupt mode
	EIMSK |= (1 << INT1) | (1 << INT0); //Enable Interrupts
	
	sei(); //Interrupts ON
	
	while(1)
	{
		
	}
	return 0;
}