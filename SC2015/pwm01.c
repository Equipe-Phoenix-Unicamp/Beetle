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
	if (PIND & (1 << PD2))
	{
		ch0_last = TCNT2 + timovf_count*256;
	}
	else
	{
		ch0 = (TCNT2 + timovf_count*256) - ch0_last;
	}
}

ISR(INT1_vect)
{
	if (PIND & (1 << PD3))
	{
		ch1_last = TCNT2 + timovf_count*256;
	}
	else
	{
		ch1 = (TCNT2 + timovf_count*256) - ch1_last;
	}

}

ISR(TIMER2_OVF_vect)
{
	timovf_count++;
}

int main(void)
{
	CLKPR = 0; //Sets internal RC oscilator clock to 8MHz
	
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01); // Configures PWM on pin OC0A and OC0B
	TCCR0B |= (1 << CS00);
	
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10); // Configures PWM on pin OC1A and OC1B
	TCCR1B |= (1 << WGM12) | (1 << CS10); 
	
	DDRD |= (1 << PD6) | (1 << PD5); //OC0A and OC0B
	DDRB |= (1 << PB2) | (1 << PB1); //OC1A and OC1B
	DDRB |= (1 << PB5) | (1 << PB7); //RESETS
	
	DDRB &= ~(1 << PB4) & ~(1 << PB3); //Error pins
	DDRD &= ~(1 << PD3) & ~(1 << PD2); //Interrupt pins
	
	PORTB |= (1 << PB5) | (1 << PB7); //Set RESET to enable the driver
	
	EICRA |= (1 << ISC10) | (1 << ISC00); //Configures interrupt mode
	EIMSK |= (1 << INT1) | (1 << INT0); //Enable Interrupts
	
	TCCR2B |= (1 << CS22); //Timer2 Configuration, prescaler to 64
	TIMSK2 |= (1 << TOIE2); //Enables ovf interrupt
	
	OCR0A = 0;
	OCR0B = 0;
	TCCR0A &= ~(1 << COM0B1);
	
	OCR1A = 100;
	OCR1B = 0;
	TCCR1A &= ~(1 << COM1B1);
	
	sei(); //Interrupts ON
	
	int16_t pwm0 = 0;
	int16_t pwm1 = 0;
	while(1)
	{
		OCR0A = ch0;
	}
	while(1)
	{
		//117 is 1.5s
		//195 is 2.5s
		pwm0 = (ch0 - 156)*6.5;
		pwm1 = (ch1 - 156)*6.5;
		
		if (pwm0 < 0)
		{
			TCCR0A &= ~(1 << COM0B1);
			TCCR0A |= (1 << COM0A1);
			OCR0A = -pwm0;
		}
		else
		{
			TCCR0A &= ~(1 << COM0A1);
			TCCR0A |= (1 << COM0B1);
			OCR0B = pwm0;
			
		}
		
		if (pwm1 < 0)
		{
			TCCR1A &= ~(1 << COM1B1);
			TCCR1A |= (1 << COM1A1);
			OCR1A = -pwm1;
		}
		else
		{
			TCCR1A &= ~(1 << COM1A1);
			TCCR1A |= (1 << COM1B1);
			OCR1B = pwm1;
		}
		
	}
	return 0;
}