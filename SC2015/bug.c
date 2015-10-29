/*
 * Beetle_C.cpp
 *
 * Created: 24/10/2015 06:23:47
 *  Author: Fruxes
 */ 

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/* Macros input, output, low, high */
#define output_low(port,pin) {port &= ~(1<<pin);}
#define output_high(port,pin) {port |= (1<<pin);}
#define set_input(portdir,pin) {portdir &= ~(1<<pin);}
#define set_output(portdir,pin) {portdir |= (1<<pin);}

volatile uint16_t time_int_1 = 0;
volatile uint16_t time_int_1_before = 0;
volatile uint16_t time_int_0 = 0;
volatile uint16_t time_int_0_before = 0;

volatile int16_t last_pow_dir = 0;
volatile int16_t last_pow_esq = 0;

//********************terminando de verificar
ISR(INT0_vect){ // Interrupcao chamada quando pino muda de estado
	if (PIND & (1 << PIND3)){ //Pino foi para alto
		TCNT0 = 0;
		time_int_0_before = TCNT0; //Salva tempo
	}
	else{//Pino foi para baixo
		if ((TCNT0 - time_int_0_before) > 120 && (TCNT0 - time_int_0_before) < 255){
			time_int_0 = TCNT0 - time_int_0_before; //Salva diferenca de tempo para calculo do pwm
		}
	}
}
//********************terminando de verificar
ISR(INT1_vect){ //Vide funcao acima
	if (PIND & (1 << PIND2)){
		TCNT0 = 0;
		time_int_1_before = TCNT0;
	}
	else{
		if ((TCNT0 - time_int_1_before) > 120 && (TCNT0 - time_int_1_before) < 255){
			time_int_1 = TCNT0 - time_int_1_before;
		}
	}
}

/* Configura as portas como inputs e outputs */
void init_ports (void){
	/*Define as portas como saída*/
	set_output(DDRB, PB1); //OC1A e PWM_B
	set_output(DDRB, PB2); //OC1B e PWM_A
	set_output(DDRB, PB5); //SCK e (RESET_AB)'
	set_output(DDRB, PB7); //(RESET_CD)'
	set_output(DDRD, PD5); //PWM_C
	set_output(DDRD, PD6); //PWM_D
	
	/*Define as portas como entrada*/
	set_input(DDRB, PB3); //MOSI e (FALUT)'
	set_input(DDRB, PB4); //MISO e (OTW)'
	set_input(DDRC, PC6); //(RESET)'
	set_input(DDRD, PD2); //INT0 e CH1
	set_input(DDRD, PD3); //INT1 e CH0
	
	/*Portas PC0-PC3 podem ser usadas como sensores. Não foram implementadas*/
}

/* This is just a program that 'kills time' in a calibrated method */
void delay_ms(uint8_t ms) {
	uint16_t delay_count = 1000;
	volatile uint16_t i;
	
	while (ms != 0) {
		for (i=0; i != delay_count; i++);
		ms--;
	}
}

/* Faz o módulo de um número */
int16_t absolute(int16_t a)
{
	return (a > 0)?a:-a;
}


int main(){
	init_ports();
	
	//**********************MCUCR tem que ter?
	
	/* Configuração das Interrupções */
	EICRA |= (1 << ISC00) | (1 << ISC10); // Configura para que qualquer mudança lógica em INT0 e INT1 gere uma interrupção
	EIMSK |= (1 << INT0) | (1 << INT1); // Habilita interrupções nos pinos (mas ainda não estão ativas)

	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << CS00);
	OCR0A = 0; //Seta o TOP da contagem do PWM
	OCR0B = 0;
	
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << CS10);
	OCR1A = 0;
	OCR1B = 0;
	
	WDTCSR |= (1 << WDE) | (1 << WDP3); //Habilita watchdog com reset a cada 2 s
	
	PORTB |= (1 << PB7) | (1 << PB5); //Sets RESET_AB' and RESET_CD' HIGH to enable the driver
	
	sei(); //Ativa interrupçoes
	delay_ms(100);
	
	int16_t pwm1 = 0, pwm2 = 0; //PWM1 = canal 1, pino B
						        //PWM2 = canal 3, pino A
	int16_t direita = 0, esquerda = 0;

	time_int_0 = 200; //equivalente a zero
	time_int_1 = 200; //equivalente a zero
	
	OCR1A = 200;
	OCR0A = 50;
	
	while(1)
	{
		asm("WDR");
	}
	
	while(1){
		pwm1 = 4.85*time_int_0 - 967; //Transforma sinal recebido na range desejada, entrada varia de 125 a 250
		pwm2 = 4.85*time_int_1 - 967; 
		direita = pwm2 + pwm1;
		esquerda = pwm2 - pwm1;
		
		if (!(PINB & (1 << PINB3)) || !(PINB & (1 << PINB4)))
		{
			OCR1B = 0;
			OCR0B = 0;
			OCR1A = 0;
			OCR0A = 0;
			delay_ms(50);
		}
		
		asm("WDR");
		
		if (direita >= 0) //Gira direita sentido horario
		{
			OCR0A = (direita >= 255)?255:(direita & 0xFF);
			OCR0B = 0;
		}
		else //Gira direita sentido anti-horario
		{
			OCR0B = (direita <= -255)?0:(direita & 0xFF);
			OCR0A = 0;
		}
		if (esquerda >= 0) //Gira esquerda sentido horario
		{
			OCR1A = (esquerda >= 255)?255:(esquerda & 0xFF);
			OCR1B = 0;
		}
		else //Gira direita sentido anti-horario
		{
			OCR1B = (esquerda <= -255)?0:(esquerda & 0xFF);
			OCR1A = 0;
		}
		delay_ms(50);
	}//end while
}//end main