/* Arquivo .h para comunicação serial
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h> // Para usar uint8_t

// Variáveis globais para comunicação ISR -> Main
extern volatile uint8_t serial_response_pending;
extern volatile char serial_response_char[3];

void initUART(void);
void SerialEnviaChars(int sizeS, char* string);
void SerialEnviaString(char* str);
void SerialRecebeChars(int sizeS, char* string);
int SerialRecebeCharsNonBlocking(int sizeS, char* string);
int SerialTemDados(void);
void uart_buffer_clear(void);

// Funções para controle do estado de bloqueio
uint8_t isBlocked(void);
void setBlocked(uint8_t state);


#endif /* SERIAL_H_ */