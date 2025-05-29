//#define F_CPU 16000000UL
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "timers.h" // Assuming timers.h provides delay1ms()
#include "serial.h" // Include its own header for extern definitions

#define UART_BUFFER_SIZE 128

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile int uart_head = 0;
volatile int uart_tail = 0;

volatile uint8_t blocked = 0;

// Static variables for the SH command
static char sh_command_bytes_received[6];
static uint8_t sh_bytes_count = 0;

// Static variables for general 2-character command parsing
static char first_byte_of_potential_command = 0;
static uint8_t waiting_for_second_byte = 0;

// Global flags and data for pending serial responses from ISR
volatile uint8_t serial_response_pending = 0;
volatile char serial_response_char[3]; // e.g., {'C', 'H', '\0'}


void initUART(void) {
	UBRR0H = (51 >> 8);
	UBRR0L = 51;
	UCSR0A = 0;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	sei(); // Ensure global interrupts are enabled after UART init
}

void SerialEnviaChars(int sizeS, char* string) {
	for (int i = 0; i < sizeS; i++) {
		while (!(UCSR0A & (1 << UDRE0))); // Wait for transmit buffer to be empty
		UDR0 = string[i];
		delay1ms(20); // Retained for physical Arduino compatibility as requested
	}
	while (!(UCSR0A & (1 << TXC0))); // Wait for the entire transmission to complete
	UCSR0A |= (1 << TXC0); // Clear the transmit complete flag
}

void SerialEnviaString(char* str) {
	SerialEnviaChars(strlen(str), str);
}

ISR(USART_RX_vect) {
	char received_byte = UDR0;

	// Handle 'SH' command continuation
	if (sh_bytes_count > 0 && sh_bytes_count < 6) {
		sh_command_bytes_received[sh_bytes_count++] = received_byte;
		if (sh_bytes_count == 6) {
			uint8_t hora = sh_command_bytes_received[4];
			blocked = (hora <= 8 || hora >= 20); // Update blocked status
			// Prepare response for main loop to send
			serial_response_char[0] = 'C';
			serial_response_char[1] = 'H';
			serial_response_char[2] = '\0';
			serial_response_pending = 1; // Signal main loop
			sh_bytes_count = 0;
		}
		return;
	}

	// Handle new potential 2-character commands or general data
	if (waiting_for_second_byte) {
		if (first_byte_of_potential_command == 'S') {
			if (received_byte == 'T') {
				blocked = 1;
				// Prepare response for main loop to send
				serial_response_char[0] = 'C';
				serial_response_char[1] = 'T';
				serial_response_char[2] = '\0';
				serial_response_pending = 1; // Signal main loop
				} else if (received_byte == 'L') {
				blocked = 0;
				// Prepare response for main loop to send
				serial_response_char[0] = 'C';
				serial_response_char[1] = 'L';
				serial_response_char[2] = '\0';
				serial_response_pending = 1; // Signal main loop
				} else if (received_byte == 'H') {
				sh_command_bytes_received[0] = 'S';
				sh_command_bytes_received[1] = 'H';
				sh_bytes_count = 2;
				} else {
				// Not a special 'S' command, push both to buffer
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
			// First byte was not 'S', push both to buffer
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
		// First byte received, check if it's 'S'
		if (received_byte == 'S') {
			first_byte_of_potential_command = received_byte;
			waiting_for_second_byte = 1;
			} else {
			// Not a special command start, push to buffer
			int next_head = (uart_head + 1) % UART_BUFFER_SIZE;
			if (next_head != uart_tail) {
				uart_buffer[uart_head] = received_byte;
				uart_head = next_head;
			}
		}
	}
}

int isBlocked() {
	return blocked;
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
	const int timeout_limit = 100; // 100 * 10ms = 1 second timeout

	while (received < sizeS && timeout_count < timeout_limit) {
		char c;
		if (uart_buffer_read_char(&c)) {
			string[received++] = c;
			timeout_count = 0; // Reset timeout on successful read
			} else {
			_delay_ms(10); // Wait for data
			timeout_count++;
		}
	}
	string[received] = '\0'; // Null-terminate the received string
}

int SerialRecebeCharsNonBlocking(int sizeS, char* string) {
	int bytes_recebidos = 0;
	int timeout_count = 0;
	const int timeout_limit = 50; // 50 * 10ms = 0.5 second timeout

	while (bytes_recebidos < sizeS && timeout_count < timeout_limit) {
		char c;
		if (uart_buffer_read_char(&c)) {
			string[bytes_recebidos++] = c;
			timeout_count = 0; // Reset timeout on successful read
			} else {
			_delay_ms(10); // Wait for data
			timeout_count++;
		}
	}

	if (bytes_recebidos < sizeS) {
		string[bytes_recebidos] = '\0';
		} else {
		string[sizeS] = '\0'; // Ensure it's null-terminated even if full
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