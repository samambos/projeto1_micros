#define F_CPU 16000000UL
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>

// Inicializa a UART com 19200 bps, 8N1
void initUART(void) {
    // Baud rate = 19200, UBRR = 51 para F_CPU = 16MHz
    UBRR0H = (51 >> 8);
    UBRR0L = 51;

    UCSR0A = 0; // Padrão

    // Ativa transmissor e receptor
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // 8 bits, sem paridade, 1 stop bit (8N1)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
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

// Recebe exatamente `sizeS` caracteres e termina com \0
void SerialRecebeChars(int sizeS, char* string) {
    for (int i = 0; i < sizeS; i++) {
        while (!(UCSR0A & (1 << RXC0))); // Espera byte
        string[i] = UDR0;
    }
    string[sizeS] = '\0'; // finaliza string
}

// Recebe até `sizeS` caracteres, retorna o número de bytes realmente lidos
int SerialRecebeCharsNonBlocking(int sizeS, char* string) {
	int bytes_recebidos = 0;
	for (int i = 0; i < sizeS; i++) {
		if (UCSR0A & (1 << RXC0)) { // Verifica se há um byte disponível
			string[i] = UDR0;
			bytes_recebidos++;
			} else {
			break; // Sai do loop se não houver mais bytes disponíveis
		}
	}
	if (bytes_recebidos < sizeS) { // Se não leu todos os bytes esperados, garante null-termination
		string[bytes_recebidos] = '\0';
		} else { // Se leu todos os bytes esperados, garante null-termination no final
		string[sizeS] = '\0';
	}
	return bytes_recebidos;
}