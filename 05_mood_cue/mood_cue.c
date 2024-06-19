#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h>

void setup_input() {
    ADMUX =  (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    DIDR0 |= (1 << ADC5D);
}

void start_conversion() {
    ADCSRA |= (1 << ADSC);
}

uint16_t read_channel(uint8_t pin) {
    ADMUX &= 0xF0; //clear the last 4 bits
    ADMUX |= pin;
    start_conversion();
    while(bit_is_set(ADCSRA, ADSC));
    return ADC;
}

void setup_output() {
    DDRB |= 0x2; //set bit PB1 to output
}

int main() {

    setup_input();
    setup_output();

    TCCR1A |= (0 << WGM12) | (1 << WGM11) | (0 << WGM10) | (1 << COM1A1) | (0 << COM1A0);
    ICR1 = 156;
    OCR1A = 0x01;
    TIMSK1 |= (1 << OCIE1A);

    TCCR1B |= (1 << WGM13) | (0x5);

    while(1) {

	OCR1A = read_channel(0x05)/50; 
    }

    return 0;
}
