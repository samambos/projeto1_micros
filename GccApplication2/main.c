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

// Estados do sistema
typedef enum {
	STANDBY,
	AUTENTICACAO,
	MENU,
	OPERACAO,
	ERROR_STATE
} Estado;

Estado estado_atual = STANDBY;

void aguardar_desbloqueio() {
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "    FORA  DE");
	LCD_Escrever_Linha(1, 0, "    OPERACAO");
	while (isBlocked()) {
		delay1ms(10);
	}
	LCD_limpar();
	mensagem_Inicial();
}

int ler_codigo_aluno(char* codigo) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite codigo:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		if (isBlocked()) {
			aguardar_desbloqueio();
			return 0;
		}
		tecla = varredura();
		if (tecla >= '0' && tecla <= '9') {
			codigo[pos] = tecla;
			char temp[2] = { tecla, '\0' };
			LCD_Escrever_Linha(1, pos, temp);
			pos++;
			delay1ms(200);
		}
	}
	codigo[6] = '\0';
	return 1;
}

int ler_senha(char* senha) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite senha:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		if (isBlocked()) {
			aguardar_desbloqueio();
			return 0;
		}
		tecla = varredura();
		if (tecla >= '0' && tecla <= '9') {
			senha[pos] = tecla;
			LCD_Escrever_Linha(1, pos, "*");
			pos++;
			delay1ms(200);
		}
	}
	senha[6] = '\0';
	return 1;
}

int validar_codigo_aluno(const char* codigo, const char* senha) {
	if (strlen(codigo) != 6 || strlen(senha) != 6) return 0;

	char mensagem[14];
	mensagem[0] = 'C';
	mensagem[1] = 'E';
	memcpy(&mensagem[2], codigo, 6);
	memcpy(&mensagem[8], senha, 6);
	SerialEnviaChars(14, mensagem);

	char resposta[19];
	SerialRecebeChars(18, resposta);
	resposta[18] = '\0';

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Resp Servidor:");
	LCD_Escrever_Linha(1, 0, resposta);
	delay1ms(2000);
	LCD_limpar();

	if (resposta[0] == 'S' && resposta[1] == 'E') {
		if (strstr(resposta, "Nao autorizado") != NULL) {
			return 0;
			} else {
			return 1;
		}
	}
	return 0;
}

void atualiza_estado(Estado proximo) {
	if (isBlocked()) {
		estado_atual = ERROR_STATE;
		return;
	}
	if (estado_atual == ERROR_STATE) {
		estado_atual = STANDBY;
		return;
	}
	estado_atual = proximo;
}

int main(void) {
	prepara_teclado();
	LCD_iniciar();
	initUART();

	char codigo[7];
	char senha[7];
	const char* opcoes[] = { "1-Saque", "2-Deposito", "3-Pagamento", "4-Saldo" };
	int indice = 0;
	char tecla = 0;

	while (1) {
		if (isBlocked()) estado_atual = ERROR_STATE;

		switch (estado_atual) {
			case ERROR_STATE:
			aguardar_desbloqueio();
			atualiza_estado(STANDBY);
			break;

			case STANDBY:
			LCD_limpar();
			mensagem_Inicial();
			while (varredura() == 0) {
				if (isBlocked()) {
					estado_atual = ERROR_STATE;
					break;
				}
			}
			if (estado_atual != ERROR_STATE) atualiza_estado(AUTENTICACAO);
			break;

			case AUTENTICACAO:
			if (!ler_codigo_aluno(codigo)) {
				estado_atual = STANDBY;
				break;
			}
			if (!ler_senha(senha)) {
				estado_atual = STANDBY;
				break;
			}
			if (validar_codigo_aluno(codigo, senha)) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Codigo valido!");
				LCD_Escrever_Linha(1, 0, "Processando...");
				delay1ms(2000);
				atualiza_estado(MENU);
				} else {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Conta invalida!");
				LCD_Escrever_Linha(1, 0, "Tente novamente");
				delay1ms(2000);
				atualiza_estado(STANDBY);
			}
			break;

			case MENU:
			indice = 0;
			while (estado_atual == MENU) {
				if (isBlocked()) {
					estado_atual = ERROR_STATE;
					break;
				}

				LCD_limpar();
				LCD_Escrever_Linha(0, 0, opcoes[indice]);
				if (indice + 1 < 4)
				LCD_Escrever_Linha(1, 0, opcoes[indice + 1]);

				while ((tecla = varredura()) == 0) {
					if (isBlocked()) {
						estado_atual = ERROR_STATE;
						break;
					}
				}
				if (estado_atual == ERROR_STATE) break;
				delay1ms(300);

				if (tecla == 'B' && indice < 2) indice++;
				else if (tecla == 'A' && indice > 0) indice--;
				else if (tecla == '*') {
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "Voltando...");
					delay1ms(1000);
					atualiza_estado(STANDBY);
					break;
					} else if (tecla == opcoes[indice][0]) {
					switch (tecla) {
						case '1': realizar_saque(); break;
						case '2':
						case '3':
						case '4':
						LCD_limpar();
						LCD_Escrever_Linha(0, 0, "Funcao");
						LCD_Escrever_Linha(1, 0, "Nao disponivel");
						delay1ms(2000);
						break;
					}
				}
			}
			break;

			default:
			estado_atual = STANDBY;
			break;
		}
	}
	return 0;
}
