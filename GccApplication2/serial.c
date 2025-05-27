#define F_CPU 16000000UL
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define UART_BUFFER_SIZE 128

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile int uart_head = 0; // posição onde ISR coloca próximo byte
volatile int uart_tail = 0; // posição do próximo byte para leitura na função SerialRecebeChars
volatile char comando_buffer[2];
volatile uint8_t blocked = 0;
volatile uint8_t comando_index = 0;


// Inicializa a UART com 19200 bps, 8N1
void initUART(void) {
	// Baud rate = 19200, UBRR = 51 para F_CPU = 16MHz
	UBRR0H = (51 >> 8);
	UBRR0L = 51;

	UCSR0A = 0; // Padrão

	// Ativa transmissor e receptor e interrupção RX completa
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

	// 8 bits, sem paridade, 1 stop bit (8N1)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

	sei(); // habilita interrupções globais
}

// Envia uma string de tamanho definido
void SerialEnviaChars(int sizeS, char* string) {
	for (int i = 0; i < sizeS; i++) {
		while (!(UCSR0A & (1 << UDRE0))); // Espera registrador livre
		UDR0 = string[i];
	}

	// Espera a transmissão do último byte
	while (!(UCSR0A & (1 << TXC0)));
	UCSR0A |= (1 << TXC0); // Limpa flag TXC
}

// Envia string null-terminated (mais comum)
void SerialEnviaString(char* str) {
	SerialEnviaChars(strlen(str), str);
}

// Buffer circular auxiliar para receber dados na ISR
ISR(USART_RX_vect) {
	char received = UDR0;

	// Comando de 2 bytes: ST ou SL
	comando_buffer[comando_index++] = received;

	if (comando_index >= 2) {
		if (comando_buffer[0] == 'S' && comando_buffer[1] == 'T') {
			blocked = 1;
			SerialEnviaString("CT");
			} else if (comando_buffer[0] == 'S' && comando_buffer[1] == 'L') {
			blocked = 0;
			SerialEnviaString("CL");
			} else {
			// Não é comando especial, joga os bytes no buffer normal
			int next_head = (uart_head + 1) % UART_BUFFER_SIZE;
			if (next_head != uart_tail) {
				uart_buffer[uart_head] = comando_buffer[0];
				uart_head = next_head;
			}
			next_head = (uart_head + 1) % UART_BUFFER_SIZE;
			if (next_head != uart_tail) {
				uart_buffer[uart_head] = comando_buffer[1];
				uart_head = next_head;
			}
		}
		comando_index = 0; // reseta para próxima leitura
	}
}

int isBlocked(){
	if (blocked){
		return 1;
	}else{
		return 0;
	}
}

// Função auxiliar para verificar se buffer está vazio
int uart_buffer_empty() {
	return (uart_head == uart_tail);
}

// Função auxiliar para ler um byte do buffer
int uart_buffer_read_char(char *c) {
	if (uart_buffer_empty()) {
		return 0; // nada para ler
	}
	*c = uart_buffer[uart_tail];
	uart_tail = (uart_tail + 1) % UART_BUFFER_SIZE;
	return 1;
}

// Recebe exatamente `sizeS` caracteres e termina com \0, com timeout (~1s)
// Timeout é em número de loops com delay curto (aprox. 10ms por loop)
void SerialRecebeChars(int sizeS, char* string) {
	int received = 0;
	int timeout_count = 0;
	const int timeout_limit = 100; // ~1 segundo timeout (100 * 10ms)

	while (received < sizeS && timeout_count < timeout_limit) {
		char c;
		if (uart_buffer_read_char(&c)) {
			string[received++] = c;
			timeout_count = 0; // reinicia timeout ao receber caractere
			} else {
			_delay_ms(10);
			timeout_count++;
		}
	}
	string[received] = '\0';
}

// Recebe até `sizeS` caracteres, retorna o número de bytes realmente lidos, com timeout (~500ms)
int SerialRecebeCharsNonBlocking(int sizeS, char* string) {
	int bytes_recebidos = 0;
	int timeout_count = 0;
	const int timeout_limit = 50; // ~500ms (50 * 10ms)

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

