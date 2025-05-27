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
#include "serial.h" // Certifique-se de ter a implementação da serial separada aqui, se não, inclua diretamente

volatile int terminal_travado = 0; // 1 = fora de operação

// Verifica comandos recebidos do servidor e atualiza estado do terminal
void verifica_comandos_servidor() {
	char comando[3];
	if (SerialRecebeCharsNonBlocking(2, comando) == 2) {
		comando[2] = '\0';

		if (strcmp(comando, "ST") == 0) {
			terminal_travado = 1;
			SerialEnviaString("CT");
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "    FORA  DE");
			LCD_Escrever_Linha(1, 0, "    OPERACAO");
		}
		else if (strcmp(comando, "SL") == 0) {
			terminal_travado = 0;
			SerialEnviaString("CL");
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "     CAIXA");
			LCD_Escrever_Linha(1, 0, "   LIBERADO!");
			delay1ms(1500);
		}
	}
}

void ler_codigo_aluno(char* codigo) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite codigo:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		if (terminal_travado) return;

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

void ler_senha(char* senha) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite senha:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		if (terminal_travado) return;

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


// Valida código com o servidor via serial
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

int main(void) {
	prepara_teclado();
	LCD_iniciar();
	initUART();

	char codigo_aluno[7];
	char senha_aluno[7];

	const char* opcoes[] = {
		"1-Saque",
		"2-Deposito",
		"3-Pagamento",
		"4-Saldo"
	};
	const int total_opcoes = 4;
	int indice_menu = 0;
	char tecla;

	while (1) {
		verifica_comandos_servidor();

		if (terminal_travado) {
			delay1ms(500);
			continue;
		}

		LCD_limpar();
		mensagem_Inicial();

		while (varredura() == 0) {
			verifica_comandos_servidor();
			if (terminal_travado) break;
		}
		if (terminal_travado) continue;

		ler_codigo_aluno(codigo_aluno);
		if (terminal_travado) continue; 
		ler_senha(senha_aluno);
		if (terminal_travado) continue; 

		if (validar_codigo_aluno(codigo_aluno, senha_aluno)) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Codigo valido!");
			LCD_Escrever_Linha(1, 0, "Processando...");
			delay1ms(2000);

			int menu_ativo = 1;
			indice_menu = 0;

			while (menu_ativo) {
				verifica_comandos_servidor();
				if (terminal_travado) break;

				LCD_limpar();
				LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
				if (indice_menu + 1 < total_opcoes) {
					LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
					} else {
					LCD_Escrever_Linha(1, 0, " ");
				}

				while ((tecla = varredura()) == 0) {
					verifica_comandos_servidor();
					if (terminal_travado) break;
				}
				if (terminal_travado) break;

				delay1ms(300);

				if (tecla == 'B' && indice_menu < total_opcoes - 2) {
					indice_menu++;
					} else if (tecla == 'A' && indice_menu > 0) {
					indice_menu--;
					} else if (tecla == '*') {
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "Voltando...");
					delay1ms(1000);
					menu_ativo = 0;
					} else if (tecla == opcoes[indice_menu][0]) {
					switch (tecla) {
						case '1':
						realizar_saque();
						break;
						case '2':
						LCD_limpar();
						LCD_Escrever_Linha(0, 0, "Deposito");
						LCD_Escrever_Linha(1, 0, "Em desenvolvimento");
						delay1ms(2000);
						break;
						case '3':
						LCD_limpar();
						LCD_Escrever_Linha(0, 0, "Pagamento");
						LCD_Escrever_Linha(1, 0, "Em desenvolvimento");
						delay1ms(2000);
						break;
						case '4':
						LCD_limpar();
						LCD_Escrever_Linha(0, 0, "Saldo");
						LCD_Escrever_Linha(1, 0, "Em desenvolvimento");
						delay1ms(2000);
						break;
					}
				}
			}
			} else {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Conta invalida!");
			LCD_Escrever_Linha(1, 0, "Tente novamente");
			delay1ms(2000);
		}
	}

	return 0;
}
