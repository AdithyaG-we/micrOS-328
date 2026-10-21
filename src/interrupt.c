#include "interrupt.h"

void enable_INT0(void){
    cli();
    EIMSK |= (1 << INT0);
    EICRA |= (uint8_t)2; // falling edge trigger
    PORTD |= (1 << PD2); // enable pull-up resistor
    sei();
}

void enable_INT1(void){
    cli();
    EIMSK |= (1 << INT1);
    EICRA |= (uint8_t)8; // falling edge trigger
    PORTD |= (1 << PD3); // enable pull-up resistor
    sei();
}

void disable_INT0(void){
    cli();
    EIMSK &= ~(1 << INT0);
    sei();
}

void disable_INT1(void){
    cli();
    EIMSK &= ~(1 << INT1);
    sei();
}
