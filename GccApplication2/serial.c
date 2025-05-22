/* Biblioteca para comunica��o serial
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Configura��es para comunica��o serial
void initUART() {
	
	// Definindo baud rate de 19200
	UBRR0H = 0;
	UBRR0L = 51;

	// Ativa o receptor
	UCSR0B = (1 << 4);

	// Formato 8 bits com 1 stop bit
	UCSR0C = 0x06;
}

// Fun��o para envio de serial
void SerialEnviaChars(int sizeS, char* string) {
	
	UCSR0B |= (1 << TXEN0);  // habilita transmissor 
	for (int i = 0; i < sizeS; i++) {  // Envia um byte de cada vez
		while (!(UCSR0A & (1 << UDRE0))) {
			// Aguarda a flag que indica que esta vazio
		}
		UDR0 = string[i];  // Coloca o byte a ser transmitido no registrador de dados
	}
	// Aguarda o �ltimo byte ser transmitido completamente
	while (!(UCSR0A & (1 << TXC0))) ;// Aguarda o bit TXC0 (Transmission Complete) ser definido
	
	UCSR0A |= (1 << TXC0);  //Limpa o TXC0 para transmiss�es futuras
}

// RECEBE MENSAGENS POR SERIAL /////////////////
void SerialRecebeChars(int sizeS, char* string){  // sizeS � o numero de bytes que ser�o recebidos | string � onde os bytes recebidos ser�o armazenados
	UCSR0B = (1 << 4); // ativa receptor
	int i;
	int count = 0;
	for(i = 0; i < sizeS; i++){  // loop que recebe, byte por byte, os dados por serial
		while (!(UCSR0A & (1<<RXC0))); // faz o c�digo esperar a chegada do byte
		string[i] =(char) UDR0; // byte que chegou do serial � armazenado em UDR0 e copiado para string[i]
	}
	string[sizeS] = '\0'; // adiciona no final da string
}