#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "timers.h"
#include "LCD.h"
#include "teclado.h"
#include "caixa_inicial.h"
#include "operacao.h"
#include "serial.h"
#include "login.h"

// Definições para o controle de timeout e LED
#define LED_PIN PB4        // Pino do LED (PB4 = pino 12 no Arduino Uno)
#define TIMEOUT_TOTAL 30000 // 30 segundos em milissegundos
#define TIMEOUT_ALERTA 18000 // 18 segundos (30s - 12s) para começar a piscar
#define INTERVALO_PISCA 250  // 250ms para piscar 2 vezes por segundo (500ms período)

typedef enum {
	ESTADO_TELA_INICIAL,
	ESTADO_CODIGO,
	ESTADO_SENHA,
	ESTADO_VALIDACAO,
	ESTADO_MENU,
	ESTADO_SAQUE,
	ESTADO_PAGAMENTO,
	ESTADO_SALDO
} Estado;

// Variáveis globais para controle do tempo e LED
volatile uint32_t timer_count = 0;
volatile uint8_t alerta_led = 0; // Flag para controle do LED
volatile uint8_t led_state = 0;  // Estado atual do LED

// Protótipos das funções
void enviar_confirmacao_operacional();
void configurar_timer();
void resetar_timeout();

// Configuração do Timer para interrupção periódica
void configurar_timer() {
	// Configurar o pino do LED como saída
	DDRB |= (1 << LED_PIN);
	PORTB &= ~(1 << LED_PIN); // Inicia com LED desligado
	
	// Configurar o Timer1 para gerar interrupção a cada 1ms
	TCCR1A = 0; // Modo normal
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS11); // CTC mode, prescaler 64
	OCR1A = 250; // Valor para interrupção a cada 1ms (16MHz/64/250 = 1ms)
	TIMSK1 = (1 << OCIE1A); // Habilitar interrupção por comparação
}

ISR(TIMER1_COMPA_vect) {
	timer_count++;
	
	// Verifica se está no período de alerta (últimos 12 segundos)
	if (timer_count >= TIMEOUT_ALERTA && timer_count < TIMEOUT_TOTAL) {
		alerta_led = 1;
		
		// Piscar o LED a cada 250ms (2 vezes por segundo)
		if (timer_count % INTERVALO_PISCA == 0) {
			led_state = !led_state;
			if (led_state) {
				PORTB |= (1 << LED_PIN);  // Liga LED
				} else {
				PORTB &= ~(1 << LED_PIN); // Desliga LED
			}
		}
	}
	
	// Timeout completo (30 segundos)
	if (timer_count >= TIMEOUT_TOTAL) {
		enviar_confirmacao_operacional();
		finalizar_sessao();
		timer_count = 0;
		alerta_led = 0;
		PORTB &= ~(1 << LED_PIN); // Desliga LED
	}
}

// Função para enviar a mensagem de confirmação operacional
void enviar_confirmacao_operacional() {
	char confirmacao[2];
	confirmacao[0]='C';
	confirmacao[1]='O';
	SerialEnviaChars(2, confirmacao);
}

// Função para resetar o contador de timeout
void resetar_timeout() {
	timer_count = 0;
	alerta_led = 0;
	PORTB &= ~(1 << LED_PIN); // Desliga LED
}

void aguardar_desbloqueio() {
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "    FORA  DE");
	LCD_Escrever_Linha(1, 0, "    OPERACAO");
	while (isBlocked()) {
		delay1ms(200);
	}
	LCD_limpar();
}

int main(void) {
	prepara_teclado();
	LCD_iniciar();
	initUART();
	configurar_timer();
	sei(); // Habilitar interrupções globais

	char codigo_aluno[7];
	char senha_aluno[7];

	const char* opcoes[] = {
		"1-Saque",
		"2-Pagamento",
		"3-Saldo",
		"4-Sair"
	};
	const int total_opcoes = 4;
	int indice_menu = 0;
	char tecla;

	Estado estado = ESTADO_TELA_INICIAL;

	while (1) {
		if (isBlocked()) {
			aguardar_desbloqueio();
			estado = ESTADO_TELA_INICIAL;
			continue;
		}

		switch (estado) {
			case ESTADO_TELA_INICIAL:
			resetar_timeout();
			LCD_limpar();
			mensagem_Inicial();
			while (varredura() == 0) {
				if (isBlocked()) break;
			}
			if (!isBlocked()) estado = ESTADO_CODIGO;
			break;

			case ESTADO_CODIGO:
			resetar_timeout();
			ler_codigo_aluno(codigo_aluno);
			if (!isBlocked()) estado = ESTADO_SENHA;
			break;

			case ESTADO_SENHA:
			resetar_timeout();
			ler_senha(senha_aluno);
			if (!isBlocked()) estado = ESTADO_VALIDACAO;
			break;

			case ESTADO_VALIDACAO:
			resetar_timeout();
			if (validar_codigo_aluno(codigo_aluno, senha_aluno)) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "BEM VINDO(A)!");
				LCD_Escrever_Linha(1, 0, "PROCESSANDO...");
				delay1ms(2000);
				estado = ESTADO_MENU;
				} else {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "CONTA INVALIDA!");
				LCD_Escrever_Linha(1, 0, "TENTE NOVAMENTE");
				delay1ms(2000);
				estado = ESTADO_TELA_INICIAL;
			}
			break;

			case ESTADO_MENU:
			resetar_timeout();
			// Exibe as opções iniciais do menu
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
			if (indice_menu + 1 < total_opcoes) {
				LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
				} else {
				LCD_Escrever_Linha(1, 0, " ");
			}

			while (1) { // Permanece neste loop até uma seleção ou saída válida
				if (isBlocked()) break;

				tecla = varredura();
				if (tecla != 0) {
					delay1ms(300); // Debounce delay

					if (tecla == 'B') { // Rolar para baixo
						if (indice_menu < total_opcoes - 2) { // Garante que não ultrapasse os limites para a segunda linha
							indice_menu++;
							LCD_limpar();
							LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
							if (indice_menu + 1 < total_opcoes) {
								LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
								} else {
								LCD_Escrever_Linha(1, 0, " ");
							}
						}
						} else if (tecla == 'A') { // Rolar para cima
						if (indice_menu > 0) {
							indice_menu--;
							LCD_limpar();
							LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
							LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
						}
						} else if (tecla == '*') {
						LCD_limpar();
						LCD_Escrever_Linha(0, 0, "VOLTANDO...");
						delay1ms(1000);
						estado = ESTADO_TELA_INICIAL;
						break; // Sai do loop while para mudar de estado
						} else {
						// Lida com a seleção das opções 1, 2, 3, 4
						switch (tecla) {
							case '1': estado = ESTADO_SAQUE; break;
							case '2': estado = ESTADO_PAGAMENTO; break;
							case '3': estado = ESTADO_SALDO; break;
							case '4':
							finalizar_sessao();
							estado = ESTADO_TELA_INICIAL;
							break;
						}
						break; // Sai do loop while para mudar de estado
					}
				}
			}
			break; // Sai do switch case para ESTADO_MENU

			case ESTADO_SAQUE:
			resetar_timeout();
			realizar_saque();
			estado = ESTADO_MENU;
			break;

			case ESTADO_PAGAMENTO:
			resetar_timeout();
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Pagamento");
			LCD_Escrever_Linha(1, 0, "Em desenvolvimento");
			delay1ms(2000);
			estado = ESTADO_MENU;
			break;

			case ESTADO_SALDO:
			resetar_timeout();
			consultar_saldo();
			estado = ESTADO_MENU;
			break;
		}
	}

	return 0;
}