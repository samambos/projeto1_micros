#define F_CPU 16000000UL
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define UART_BUFFER_SIZE 128

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile int uart_head = 0;
volatile int uart_tail = 0;
volatile char comando_buffer[2];
volatile uint8_t blocked = 0;
volatile uint8_t comando_index = 0;

void initUART(void) {
	UBRR0H = (51 >> 8);
	UBRR0L = 51;
	UCSR0A = 0;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	sei();
}

void SerialEnviaChars(int sizeS, char* string) {
	for (int i = 0; i < sizeS; i++) {
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = string[i];
	}
	while (!(UCSR0A & (1 << TXC0)));
	UCSR0A |= (1 << TXC0);
}

void SerialEnviaString(char* str) {
	SerialEnviaChars(strlen(str), str);
}

ISR(USART_RX_vect) {
    static char buffer_temp[6];
    static uint8_t temp_index = 0;
    char received = UDR0;

    // Caso esteja esperando o complemento do comando "SH"
    if (temp_index > 0 && temp_index < 6) {
        buffer_temp[temp_index++] = received;
        if (temp_index == 6) {
            uint8_t hora = buffer_temp[4];
            blocked = (hora < 8 || hora >= 20);
            SerialEnviaString("CH");
            temp_index = 0;
        }
        return;
    }

    // Comando normal de 2 caracteres
    comando_buffer[comando_index++] = received;

    if (comando_index == 2) {
        if (comando_buffer[0] == 'S' && comando_buffer[1] == 'T') {
            blocked = 1;
            SerialEnviaString("CT");
        } else if (comando_buffer[0] == 'S' && comando_buffer[1] == 'L') {
            blocked = 0;
            SerialEnviaString("CL");
        } else if (comando_buffer[0] == 'S' && comando_buffer[1] == 'H') {
            buffer_temp[0] = 'S';
            buffer_temp[1] = 'H';
            temp_index = 2;
        } else {
            for (int i = 0; i < 2; i++) {
                int next_head = (uart_head + 1) % UART_BUFFER_SIZE;
                if (next_head != uart_tail) {
                    uart_buffer[uart_head] = comando_buffer[i];
                    uart_head = next_head;
                }
            }
        }
        comando_index = 0;
    }
}


int isBlocked() {
	return blocked;
}

int uart_buffer_empty() {
	return (uart_head == uart_tail);
}

int uart_buffer_read_char(char *c) {
	if (uart_buffer_empty()) return 0;
	*c = uart_buffer[uart_tail];
	uart_tail = (uart_tail + 1) % UART_BUFFER_SIZE;
	return 1;
}

void SerialRecebeChars(int sizeS, char* string) {
	uart_buffer_clear();
	int received = 0;
	int timeout_count = 0;
	const int timeout_limit = 100;

	while (received < sizeS && timeout_count < timeout_limit) {
		char c;
		if (uart_buffer_read_char(&c)) {
			string[received++] = c;
			timeout_count = 0;
			} else {
			_delay_ms(10);
			timeout_count++;
		}
	}
	string[received] = '\0';
}

int SerialRecebeCharsNonBlocking(int sizeS, char* string) {
	uart_buffer_clear();
	int bytes_recebidos = 0;
	int timeout_count = 0;
	const int timeout_limit = 50;

	while (bytes_recebidos < sizeS && timeout_count < timeout_limit) {
		char c;
		if (uart_buffer_read_char(&c)) {
			string[bytes_recebidos++] = c;
			timeout_count = 0;
			} else {
			_delay_ms(10);
			timeout_count++;
		}
	}

	if (bytes_recebidos < sizeS) {
		string[bytes_recebidos] = '\0';
		} else {
		string[sizeS] = '\0';
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
