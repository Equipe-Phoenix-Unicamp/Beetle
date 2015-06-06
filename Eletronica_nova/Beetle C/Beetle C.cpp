/*
 * Beetle_C.cpp
 *
 * Created: 29/05/2015 20:21:04
 *  Author: Fruxes
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

// Macros input, output, low, high
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

ISR(INT0_vect){ //Interrupcao chamada quando pino muda de estado
	if (PINB & (1 << PINB6)) //Pino foi para alto
	{
		time_int_0_before = TCNT0L; //Salva tempo
	}
	else //Pino foi para baixo
	{
		if ((TCNT0L - time_int_0_before) > 120 && (TCNT0L - time_int_0_before) < 255) 
		{
			time_int_0 = TCNT0L - time_int_0_before; //Salva diferenca de tempo para calculo do pwm
		}
	}
}

ISR(INT1_vect){ //Vide funcao acima
	if (PINA & (1 << PINA2))	
	{
		time_int_1_before = TCNT0L;
	}
	else
	{
		if ((TCNT0L - time_int_1_before) > 120 && (TCNT0L - time_int_1_before) < 255)
		{
			time_int_1 = TCNT0L - time_int_1_before;
		}
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

int16_t absolute(int16_t a)
{
	return (a > 0)?a:-a;
}

//INT0 PB6     INT1 PA2
int main(){
	configura_portas();
	
	MCUCR |= (1 << ISC00) | (0 << ISC01); //Configura modo das interrupcoes
	GIMSK |= (1 << INT0) | (1 << INT1); //Hobilita interrupcoes nos pinos (mas ainda nao estao ativas)

	TCCR0B |= (1 << CS00) | (1 << CS01); //Configura timer que ira ler o sinal do receptor (pre scaler 64)

	TCCR1A |= (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0) | (1 << PWM1A) | (1 << PWM1B); // Configura PWM modo nao inversor, apenas PWM, sem o barrado
	TCCR1B |= (1 << CS10); //Configura timer do gerador de PWM
	TCCR1D |= (1 << WGM10); //Configura como FAST pwm
	OCR1C = 255; //Seta o TOP da contagem do pwm
	WDTCR |= (1 << WDE) | (0 << WDP0) | (0 << WDP1) | (0 << WDP2) | (1 << WDP3); //Habilita watchdog com reset a cada 2 s
	
	OCR1A = 0;
	OCR1B = 0;
	
	sei(); //Ativa interrupçoes
	delay_ms(100);
	
	int16_t pwm1 = 0, pwm2 = 0; //PWM1 = canal 1, pino B
						//PWM2 = canal 3, pino A
	int16_t direita = 0, esquerda = 0;
	bool safe_to_start = false;
	do{
		if (absolute(time_int_0 - 200) <= 10) //Verifica se o robo esta no zero para poder comecar
		{
			if (absolute(time_int_1 - 200) <= 10) safe_to_start = true; //If em duas linhas para organizar. Dois canais tem que estar em zero
		}
	}while (!safe_to_start);
	time_int_0 = 200; //equivalente a zero
	time_int_1 = 200; //equivalente a zero
	while(1){
		pwm1 = 4.85*time_int_0 - 967; //Transforma sinal recebido na range desejada, entrada varia de 125 a 250
		pwm2 = 4.85*time_int_1 - 967; 
		direita = pwm2 + pwm1;
		esquerda = pwm2 - pwm1;
		
		if (!(PINA & (1 << PINA1)))
		{
			OCR1B = 0;
			output_low(PORTB, PINB2);
			delay_ms(50);
		}
		
		if (!(PINA & (1 << PINA0)))
		{
			OCR1A = 0;
			output_low(PORTB, PINB0);
			delay_ms(50);
		}
		
		if (absolute(direita - last_pow_dir) <= 10 || absolute(direita-200) <= 10) //Se ficou muito tempo no mesmo sinal nao reseta WD, a menos que seja zero
		{
			last_pow_dir = direita;
			asm("WDR");
		}
		if (absolute(esquerda - last_pow_esq) <= 10 || absolute(esquerda-200) <= 10) //Se ficou muito tempo no mesmo sinal nao reseta WD, a menos que seja zero
		{
			last_pow_esq = esquerda;
			asm("WDR");
		}
		if (direita >= 0) //Gira direita sentido horario
		{
			OCR1A = (direita >= 255)?255:(direita & 0xFF);
			output_low(PORTB, PINB0);
		}
		else //Gira direita sentido anti-horario
		{
			OCR1A = (direita <= -255)?0:(direita & 0xFF);
			output_high(PORTB, PINB0);
		}
		if (esquerda >= 0) //Gira esquerda sentido horario
		{
			OCR1B = (esquerda >= 255)?255:(esquerda & 0xFF);
			output_low(PORTB, PINB2);
		}
		else //Gira direita sentido anti-horario
		{
			OCR1B = (esquerda <= -255)?0:(esquerda & 0xFF);
			output_high(PORTB, PINB2);
		}
		delay_ms(50);
	}//end while
}//end main