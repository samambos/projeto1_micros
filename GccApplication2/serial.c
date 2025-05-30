//#define F_CPU 16000000UL
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "timers.h"
#include "serial.h"

#define UART_BUFFER_SIZE 128

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile int uart_head = 0;
volatile int uart_tail = 0;

volatile uint8_t blocked = 0; // Estado de bloqueio do ATM

static char sh_command_bytes_received[6];
static uint8_t sh_bytes_count = 0;

static char first_byte_of_potential_command = 0;
static uint8_t waiting_for_second_byte = 0;

volatile uint8_t serial_response_pending = 0;
volatile char serial_response_char[3];

// Indica se o sistema está bloqueado
uint8_t isBlocked() {
	return blocked;
}

// Sinaliza se o sistema está bloqueado ou desbloqueado
void setBlocked(uint8_t state) {
	blocked = state;
}

void initUART(void) {
	UBRR0H = (51 >> 8); // Configura Baud Rate para 19200
	UBRR0L = 51;
	UCSR0A = 0;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // Habilita RX, TX e interrupção de RX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Formato 8 bits de dados
	sei(); // Habilita interrupções globais
}

void SerialEnviaChars(int sizeS, char* string) {
	for (int i = 0; i < sizeS; i++) {
		while (!(UCSR0A & (1 << UDRE0))); // Espera o buffer de transmissão estar vazio
		UDR0 = string[i]; // Envia o byte
		delay1ms(20); // Atraso mantido para compatibilidade de hardware
	}
	while (!(UCSR0A & (1 << TXC0))); // Espera a transmissão completa
	UCSR0A |= (1 << TXC0); // Limpa a flag de transmissão completa
}

void SerialEnviaString(char* str) {
	SerialEnviaChars(strlen(str), str);
}

ISR(USART_RX_vect) {
	char received_byte = UDR0;

	// Lida com a continuação do comando 'SH'
	if (sh_bytes_count > 0 && sh_bytes_count < 6) {
		sh_command_bytes_received[sh_bytes_count++] = received_byte;
		if (sh_bytes_count == 6) {
			uint8_t hora = sh_command_bytes_received[4];
			setBlocked(hora < 8 || hora >= 20); // Bloqueia se fora do horário
			serial_response_char[0] = 'C';
			serial_response_char[1] = 'H';
			serial_response_char[2] = '\0';
			serial_response_pending = 1; // Sinaliza o loop principal
			sh_bytes_count = 0; // Reseta o contador
		}
		return;
	}

	// Lida com novos comandos de 2 caracteres ou dados gerais
	if (waiting_for_second_byte) {
		if (first_byte_of_potential_command == 'S') {
			if (received_byte == 'T') { // Comando 'ST' (Travar Terminal)
				setBlocked(1); // Bloqueia
				serial_response_char[0] = 'C';
				serial_response_char[1] = 'T';
				serial_response_char[2] = '\0';
				serial_response_pending = 1; // Sinaliza o loop principal
				} else if (received_byte == 'L') { // Comando 'SL' (Liberar Terminal)
				setBlocked(0); // Desbloqueia
				serial_response_char[0] = 'C';
				serial_response_char[1] = 'L';
				serial_response_char[2] = '\0';
				serial_response_pending = 1; // Sinaliza o loop principal
				} else if (received_byte == 'H') { // Início do comando 'SH'
				sh_command_bytes_received[0] = 'S';
				sh_command_bytes_received[1] = 'H';
				sh_bytes_count = 2; // Já recebeu os dois primeiros bytes
				} else {
				// Não é um comando 'S' especial, empurra para o buffer
				int next_head = (uart_head + 1) % UART_BUFFER_SIZE;
				if (next_head != uart_tail) {
					uart_buffer[uart_head] = first_byte_of_potential_command;
					uart_head = next_head;
				}
				next_head = (uart_head + 1) % UART_BUFFER_SIZE;
				if (next_head != uart_tail) {
					uart_buffer[uart_head] = received_byte;
					uart_head = next_head;
				}
			}
			} else {
			// Não é 'S', empurra para o buffer
			int next_head = (uart_head + 1) % UART_BUFFER_SIZE;
			if (next_head != uart_tail) {
				uart_buffer[uart_head] = first_byte_of_potential_command;
				uart_head = next_head;
			}
			next_head = (uart_head + 1) % UART_BUFFER_SIZE;
			if (next_head != uart_tail) {
				uart_buffer[uart_head] = received_byte;
				uart_head = next_head;
			}
		}
		waiting_for_second_byte = 0;
		first_byte_of_potential_command = 0;
		} else {
		// Primeiro byte recebido, verifica se é 'S'
		if (received_byte == 'S') {
			first_byte_of_potential_command = received_byte;
			waiting_for_second_byte = 1;
			} else {
			// Não é início de comando especial, empurra para o buffer
			int next_head = (uart_head + 1) % UART_BUFFER_SIZE;
			if (next_head != uart_tail) {
				uart_buffer[uart_head] = received_byte;
				uart_head = next_head;
			}
		}
	}
}

int uart_buffer_empty() {
	return (uart_head == uart_tail);
}

int uart_buffer_count() {
	if (uart_head >= uart_tail) {
		return uart_head - uart_tail;
		} else {
		return UART_BUFFER_SIZE - uart_tail + uart_head;
	}
}

int uart_buffer_read_char(char *c) {
	if (uart_buffer_empty()) return 0;
	*c = uart_buffer[uart_tail];
	uart_tail = (uart_tail + 1) % UART_BUFFER_SIZE;
	return 1;
}

void SerialRecebeChars(int sizeS, char* string) {
	int received = 0;
	int timeout_count = 0;
	const int timeout_limit = 100; // 1 segundo de timeout

	while (received < sizeS && timeout_count < timeout_limit) {
		char c;
		// Permite ao main loop enviar ACK mesmo com bloqueio
		if (serial_response_pending) {
			delay1ms(10); // Pequeno atraso para o main loop
			continue; // Tenta novamente
		}

		if (uart_buffer_read_char(&c)) {
			string[received++] = c;
			timeout_count = 0; // Reseta timeout
			} else {
			delay1ms(10); // Espera por dados
			timeout_count++;
		}
	}
	string[received] = '\0'; // Termina a string
}

int SerialRecebeCharsNonBlocking(int sizeS, char* string) {
	int bytes_recebidos = 0;
	int timeout_count = 0;
	const int timeout_limit = 50; // 0.5 segundo de timeout

	while (bytes_recebidos < sizeS && timeout_count < timeout_limit) {
		char c;
		if (uart_buffer_read_char(&c)) {
			string[bytes_recebidos++] = c;
			timeout_count = 0; // Reseta timeout
			} else {
			delay1ms(10); // Espera por dados
			timeout_count++;
		}
	}

	if (bytes_recebidos < sizeS) {
		string[bytes_recebidos] = '\0';
		} else {
		string[sizeS] = '\0'; // Garante término nulo
	}
	return bytes_recebidos;
}

int SerialTemDados() {
	return !uart_buffer_empty();
}

void uart_buffer_clear() {
	uart_head = 0;
	uart_tail = 0;
}