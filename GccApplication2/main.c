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

// Definições para controle de timeout e LED
#define LED_PIN PB4 // Pino do LED (PB4 = pino 12 no Arduino Uno)
#define TIMEOUT_TOTAL 30000 // 30 segundos em milissegundos para inatividade
#define TIMEOUT_ALERTA 18000 // 18 segundos (30s - 12s) para começar a piscar
#define INTERVALO_PISCA 250 // 250ms para piscar 2 vezes por segundo (500ms período)

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

// Variáveis globais para controle de tempo e LED
volatile uint32_t timer_count = 0;
volatile uint8_t alerta_led = 0; // Flag para controle do LED
volatile uint8_t led_state = 0; // Estado atual do LED

// Protótipos das funções
void enviar_confirmacao_operacional();
void configurar_timer();
void resetar_timeout();
void aguardar_desbloqueio();

// Configuração do Timer para interrupção periódica
void configurar_timer() {
	DDRB |= (1 << LED_PIN); // Configura pino do LED como saída
	PORTB &= ~(1 << LED_PIN); // Inicia com LED desligado

	TCCR1A = 0; // Modo normal
	TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS11); // Modo CTC, prescaler 64
	OCR1A = 250; // Valor para interrupção a cada 1ms (16MHz / 64 / 250 = 1ms)
	TIMSK1 = (1 << OCIE1A); // Habilita interrupção por comparação
}

ISR(TIMER1_COMPA_vect) {
	timer_count++;

	// Verifica período de alerta (últimos 12 segundos)
	if (timer_count >= TIMEOUT_ALERTA && timer_count < TIMEOUT_TOTAL) {
		alerta_led = 1;

		// Pisca o LED 2 vezes por segundo
		if (timer_count % INTERVALO_PISCA == 0) {
			led_state = !led_state;
			if (led_state) {
				PORTB |= (1 << LED_PIN); // Liga LED
				} else {
				PORTB &= ~(1 << LED_PIN); // Desliga LED
			}
		}
	}

	// Timeout completo (30 segundos)
	if (timer_count >= TIMEOUT_TOTAL) {
		enviar_confirmacao_operacional(); // Envia "CO"
		finalizar_sessao(); // Encerra a sessão
		timer_count = 0; // Reseta contador
		alerta_led = 0; // Desliga alerta
		PORTB &= ~(1 << LED_PIN); // Desliga LED
		setBlocked(1); // Bloqueia sistema por timeout
	}
}

// Envia mensagem de confirmação operacional "CO"
void enviar_confirmacao_operacional() {
	char confirmacao[2];
	confirmacao[0] = 'C';
	confirmacao[1] = 'O';
	SerialEnviaChars(2, confirmacao); // Envia "CO"
}

// Reseta contador de timeout e estado do LED
void resetar_timeout() {
	timer_count = 0;
	alerta_led = 0;
	PORTB &= ~(1 << LED_PIN); // Desliga LED
}

// Aguarda o desbloqueio do terminal
void aguardar_desbloqueio() {
	LCD_limpar();
	LCD_Escrever_Linha(0, 4, "FORA  DE"); // Exibe "FORA DE OPERAÇÃO"
	LCD_Escrever_Linha(1, 4, "OPERACAO");
	while (isBlocked()) { // Espera sistema ser desbloqueado
		if (serial_response_pending) {
			SerialEnviaChars(2, serial_response_char);
			serial_response_pending = 0; // Limpa a flag
			resetar_timeout(); // Reseta timeout após interação serial (desbloqueio)
		}
		delay1ms(200); // Pequeno atraso
	}
	LCD_limpar(); // Limpa LCD após desbloqueio
}

int main(void) {
	prepara_teclado();
	LCD_iniciar();
	initUART();
	configurar_timer();
	sei(); // Habilita interrupções globais

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
		// Gerencia respostas da ISR e bloqueio (crítico)
		if (serial_response_pending) {
			SerialEnviaChars(2, serial_response_char);
			serial_response_pending = 0; // Limpa a flag após envio
			resetar_timeout(); // Reseta timeout após interação serial
		}

		// Sistema bloqueado por comando ST, SH ou timeout
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
				if (isBlocked()) break; // Sai se for bloqueado
			}
			if (!isBlocked()) {
				estado = ESTADO_CODIGO;
			}
			break;

			case ESTADO_CODIGO:
			resetar_timeout();
			ler_codigo_aluno(codigo_aluno);
			if (isBlocked()) { // Se bloqueado durante a leitura
				estado = ESTADO_TELA_INICIAL;
				break;
			}
			estado = ESTADO_SENHA;
			break;

			case ESTADO_SENHA:
			resetar_timeout();
			ler_senha(senha_aluno);
			if (isBlocked()) { // Se bloqueado durante a leitura
				estado = ESTADO_TELA_INICIAL;
				break;
			}
			estado = ESTADO_VALIDACAO;
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
			LCD_limpar();
			indice_menu = 0;
			LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
			if (indice_menu + 1 < total_opcoes) {
				LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
				} else {
				LCD_Escrever_Linha(1, 0, " ");
			}

			while (1) {
				if (isBlocked()) break;

				tecla = varredura();
				if (tecla != 0) {
					delay1ms(300); // Debounce

					if (tecla == 'B') { // Rolar para baixo
						if (indice_menu < total_opcoes - 2) {
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
						} else if (tecla == '*') { // Voltar para tela inicial
						LCD_limpar();
						LCD_Escrever_Linha(0, 0, "VOLTANDO...");
						delay1ms(1000);
						estado = ESTADO_TELA_INICIAL;
						break;
						} else {
						switch (tecla) {
							case '1': estado = ESTADO_SAQUE; break;
							case '2': estado = ESTADO_PAGAMENTO; break;
							case '3': estado = ESTADO_SALDO; break;
							case '4': // Sair
							finalizar_sessao();
							estado = ESTADO_TELA_INICIAL;
							break;
						}
						break;
					}
				}
			}
			break;

			case ESTADO_SAQUE:
			resetar_timeout();
			realizar_saque();
			if (isBlocked()) { // Se bloqueado durante o saque
				estado = ESTADO_TELA_INICIAL;
				break;
			}
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
			if (isBlocked()) { // Se bloqueado durante a consulta de saldo
				estado = ESTADO_TELA_INICIAL;
				break;
			}
			estado = ESTADO_MENU;
			break;
		}
	}

	return 0;
}