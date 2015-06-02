/*
 * Beetle_C.cpp
 *
 * Created: 29/05/2015 20:21:04
 *  Author: Fruxes
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

// Macros input, output, low, high
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

volatile uint8_t time_int_1 = 0;
volatile uint8_t time_int_1_before = 0;
volatile uint8_t time_int_0 = 0;
volatile uint8_t time_int_0_before = 0;

ISR(INT0_vect){
	if (PINB & (1 << PINB6))
	{
		time_int_0_before = TCNT0L;
	}
	else
	{
		if ((TCNT0L - time_int_0_before) > 0) time_int_0 = TCNT0L - time_int_0_before;
	}
}

ISR(INT1_vect){
	if (PINA & (1 << PINA2))	
	{
		time_int_1_before = TCNT0L;
	}
	else
	{
		if ((TCNT0L - time_int_1_before) > 0) time_int_1 = TCNT0L - time_int_1_before;
	}
}

//configura os inputs e outputs
void configura_portas (){
	//define as portas como saída
	set_output(DDRB, PB0);
	set_output(DDRB, PB1);
	set_output(DDRB, PB2);
	set_output(DDRB, PB3);
	//define as portas como entrada
	set_input(DDRB, PB4);
	set_input(DDRB, PB5);
	set_input(DDRB, PB6);
	set_input(DDRB, PB7);
	set_input(DDRA, PA0);
	set_input(DDRA, PA1);
	set_input(DDRA, PA2);
}

// this is just a program that 'kills time' in a calibrated method
void delay_ms(uint8_t ms) {
	uint16_t delay_count = 1000;
	volatile uint16_t i;
	
	while (ms != 0) {
		for (i=0; i != delay_count; i++);
		ms--;
	}
}

//INT0 PB6     INT1 PA2
int main(){
	configura_portas();
	MCUCR |= (1 << ISC00) | (0 << ISC01); //MCUCR = 0b00000011
	GIMSK |= (1 << INT0) | (1 << INT1); //GISMK = 0b11000000
	delay_ms(100);

	TCCR0B |= (1 << CS00) | (1 << CS01); //Config timer

	TCCR1A |= (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0) | (1 << PWM1A) | (1 << PWM1B);
	TCCR1B |= (1 << CS10);
	TCCR1D |= (1 << WGM10);
	OCR1C = 255; //set top
	sei(); //habilita interrupçoes
	
	int16_t pwm1, pwm2;
	int16_t direita, esquerda;
	while(1){
		pwm1 = (((time_int_0 - 125) * 2.048) - 128)*2;
		pwm2 = (((time_int_1 - 125) * 2.048) - 128)*2;
		direita = pwm2 + pwm1;
		esquerda = pwm2 - pwm1;
		if (direita > 20)
		{
			OCR1A = (direita >= 255)?255:direita;
			output_low(PORTB, PINB2);
		}
		else if (direita < -20)
		{
			OCR1A = (direita <= -255)?0:(direita + 255);
			output_high(PORTB, PINB2);
		}
		else
		{
			OCR1A = 0;
			output_low(PORTB, PINB2);
		}
		if (esquerda > 20)
		{
			OCR1B = (esquerda >= 255)?255:esquerda;
			output_low(PORTB, PINB0);
		}
		else if (esquerda < -20)
		{
			OCR1B = (esquerda <= -255)?0:(esquerda + 255);
			output_high(PORTB, PINB0);
		}
		else
		{
			OCR1B = 0;
			output_high(PORTB, PINB0);
		}
		
	}//end while
}//end main