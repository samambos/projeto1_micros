/* Arquivo .h para controle de teclado matricial
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#ifndef TECLADO_H_
#define TECLADO_H_

#include <avr/io.h>

void prepara_teclado(void);  // Configura os pinos do teclado
char varredura(void);     // Faz a varredura das teclas e retorna a pressionada

#endif /* TECLADO_H_ */