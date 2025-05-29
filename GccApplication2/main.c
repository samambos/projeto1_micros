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

// Leitura do código do aluno
void ler_codigo_aluno(char* codigo) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite codigo:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		if (isBlocked()) return;
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
}

// Leitura da senha do aluno
void ler_senha(char* senha) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite senha:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		if (isBlocked()) return;
		tecla = varredura();
		if (tecla >= '0' && tecla <= '9') {
			senha[pos] = tecla;
			char temp[2] = { '*', '\0' };
			LCD_Escrever_Linha(1, pos, temp);
			pos++;
			delay1ms(200);
		}
	}
	senha[6] = '\0';
}

int validar_codigo_aluno(const char* codigo, const char* senha) {
	if (strlen(codigo) != 6 || strlen(senha) != 6) return 0;

	char mensagem[14];
	mensagem[0] = 'C';
	mensagem[1] = 'E';
	memcpy(&mensagem[2], codigo, 6);
	memcpy(&mensagem[8], senha, 6);

	SerialEnviaChars(14, mensagem);

	char resposta[32];
	memset(resposta, 0, sizeof(resposta));

	SerialRecebeChars(31, resposta);

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
			LCD_limpar();
			mensagem_Inicial();
			while (varredura() == 0) {
				if (isBlocked()) break;
			}
			if (!isBlocked()) estado = ESTADO_CODIGO;
			break;

			case ESTADO_CODIGO:
			ler_codigo_aluno(codigo_aluno);
			if (!isBlocked()) estado = ESTADO_SENHA;
			break;

			case ESTADO_SENHA:
			ler_senha(senha_aluno);
			if (!isBlocked()) estado = ESTADO_VALIDACAO;
			break;

			case ESTADO_VALIDACAO:
			if (validar_codigo_aluno(codigo_aluno, senha_aluno)) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Codigo valido!");
				LCD_Escrever_Linha(1, 0, "Processando...");
				delay1ms(2000);
				estado = ESTADO_MENU;
				} else {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Conta invalida!");
				LCD_Escrever_Linha(1, 0, "Tente novamente");
				delay1ms(2000);
				estado = ESTADO_TELA_INICIAL;
			}
			break;

			case ESTADO_MENU:
			LCD_limpar();
			indice_menu=0;
			LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
			if (indice_menu + 1 < total_opcoes)
			LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
			else
			LCD_Escrever_Linha(1, 0, " ");

			while ((tecla = varredura()) == 0) {
				if (isBlocked()) break;
			}

			delay1ms(300); // Debounce delay

			if (tecla == 'B' && indice_menu < total_opcoes - 2) {
				indice_menu++;
				} else if (tecla == 'A' && indice_menu > 0) {
				indice_menu--;
				} else if (tecla == '*') {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Voltando...");
				delay1ms(1000);
				estado = ESTADO_TELA_INICIAL;
				} else {
				switch (tecla) {
					case '1': estado = ESTADO_SAQUE; break;
					case '2': estado = ESTADO_PAGAMENTO; break;
					case '3': estado = ESTADO_SALDO; break;
					case '4':
					finalizar_sessao(); 
					estado = ESTADO_TELA_INICIAL; 
					break;
				}
			}
			break;

			case ESTADO_SAQUE:
			realizar_saque();
			estado = ESTADO_MENU;
			break;

			case ESTADO_PAGAMENTO:
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Pagamento");
			LCD_Escrever_Linha(1, 0, "Em desenvolvimento");
			delay1ms(2000);
			estado = ESTADO_MENU;
			break;

			case ESTADO_SALDO:
			consultar_saldo();
			estado = ESTADO_MENU;
			break;
		}
	}

	return 0;
}