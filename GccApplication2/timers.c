/* Biblioteca para definição de timers utilizados no projeto
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "timers.h"

void timer1hz(){ /// timer de 1 segundo
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001101;
	TIMSK1 |= (1 << 1); // habilita interrupções tmr1
	TCNT1 = 0;
	OCR1A = 16500;
}
void timer2hz(){ /// timer de 0,5 segundos (2hz) com timer2, conta até no máx 255(8 bits) para gerar 2hz é necessário colocar contador na interrupção
	TCCR2A = 0b00000000;
	TCCR2B = 0b00001101;
	TIMSK2 |= (1 << 1); // habilita interrupções tmr2
	TCNT2 = 0;
	OCR2A = 255;
}
void delay1ms(int m){ // delay de t milisegundos
	int c1;
	TCCR0A = 0b00000000;
	TCCR0B = 0b00000101;
	TIMSK0 = 0;
	TCNT0 = 241;
	for(c1 = 0 ; c1 < m ; c1++ ){
		while ((TIFR0 & (1<<1)) == 0);  /* monitor OCF0 flag */
		TCNT0 = 241;
		TIFR0 = (1<<1); /* Clear OCF0 by writing 1 */
	}
	TCCR0B = 0;
}
void delay1us(int u){ // delay de t microsegundos
	int c;
	TCCR0A = 0b00000000;
	TCCR0B = 0b00000001;
	TIMSK0 = 0;
	TCNT0 = 245;
	for(c = 0 ; c < u ; c++ ){
		while ((TIFR0 & (1<<1)) == 0);  /* monitor OCF0 flag */
		TCNT0 = 245;
		TIFR0 |= (1<<1); /* Clear OCF0 by writing 1 */
	}
	TCCR0B = 0;
}