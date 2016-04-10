#include <avr/io.h> 

int main() {
	DDRB |= (1 << DDB1);
    TCCR1A |= (1 << COM1A1) | (1 << WGM10);
    TCCR1B |= (1 << WGM12) | (1 << CS11);
    OCR1A = 250;
	
	DDRB |= (1 << DDB2);
    TCCR1A |= (1 << COM1B1) | (1 << WGM10);
    TCCR1B |= (1 << WGM12) | (1 << CS11);
    OCR1B = 50;
	
	DDRB |= (1 << DDB3);
    TCCR2A |= (1 << COM2A1) | (1 << WGM20) | (1 << WGM21);
    TCCR2B |= (1 << CS21);
    OCR2A = 50;
	
	return 0;
}