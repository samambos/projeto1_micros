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
#include "serial.h" // Adicionado para as funções SerialEnviaChars e SerialRecebeChars

// Variável global para o estado do caixa
volatile int caixa_bloqueado = 0; // 0 = liberado, 1 = bloqueado

// Leitura do código do aluno
void ler_codigo_aluno(char* codigo) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite codigo:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		tecla = varredura();
		if (tecla >= '0' && tecla <= '9') {
			codigo[pos] = tecla;
			char temp[2] = { tecla, '\0' };
			LCD_Escrever_Linha(1, pos, temp);
			pos++;
			delay1ms(200); // debounce
		}
	}
	codigo[6] = '\0'; // finaliza a string
}

// Leitura da senha
void ler_senha(char* senha) {
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite senha:");
	LCD_Escrever_Linha(1, 0, "______");

	while (pos < 6) {
		tecla = varredura();
		if (tecla >= '0' && tecla <= '9') {
			senha[pos] = tecla;
			char temp[2] = { '*', '\0' }; // Mostra asterisco no lugar
			LCD_Escrever_Linha(1, pos, temp);
			pos++;
			delay1ms(200); // debounce
		}
	}
	senha[6] = '\0'; // finaliza a string
}

// Validação com o servidor
int validar_codigo_aluno(const char* codigo, const char* senha) {
	if (strlen(codigo) != 6 || strlen(senha) != 6) return 0;

	char mensagem[14];
	mensagem[0] = 'C';
	mensagem[1] = 'E';
	memcpy(&mensagem[2], codigo, 6);
	memcpy(&mensagem[8], senha, 6);

	SerialEnviaChars(14, mensagem);

	char resposta[32];
	SerialRecebeChars(18, resposta); // recebe resposta do servidor

	if (resposta[0] == 'S' && resposta[1] == 'E') {
		return strstr(resposta, "Nao Autorizado") == NULL; // se não contiver isso, é autorizado
	}
	return 0;
}

// Verificação do status do caixa com o servidor
void verificar_status_caixa() {
	char mensagem_servidor[3];
	int bytes_lidos = SerialRecebeCharsNonBlocking(2, mensagem_servidor);

	if (bytes_lidos == 2) {
		if (mensagem_servidor[0] == 'S' && mensagem_servidor[1] == 'T') {
			caixa_bloqueado = 1;
			SerialEnviaChars(2, "CT"); // Confirmação de travamento
			} else if (mensagem_servidor[0] == 'S' && mensagem_servidor[1] == 'L') {
			caixa_bloqueado = 0;
			SerialEnviaChars(2, "CL"); // Confirmação de liberação
		}
	}
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
		"*-Saldo"
	};
	const int total_opcoes = 4;
	int estado_anterior_bloqueado = -1;
	int indice_menu = 0;
	char tecla;

	while (1) {
		verificar_status_caixa();

		if (caixa_bloqueado) {
			if (estado_anterior_bloqueado != 1) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "FORA DE OPERACAO");
				LCD_Escrever_Linha(1, 0, " ");
				estado_anterior_bloqueado = 1;
			}
			delay1ms(200);
			continue;
		}

		if (estado_anterior_bloqueado != 0) {
			LCD_limpar();
			estado_anterior_bloqueado = 0;
		}

		mensagem_Inicial();

		while (varredura() == 0) {
			verificar_status_caixa();
			if (caixa_bloqueado) break;
		}
		if (caixa_bloqueado) continue;

		ler_codigo_aluno(codigo_aluno);
		verificar_status_caixa();
		if (caixa_bloqueado) continue;

		ler_senha(senha_aluno);
		verificar_status_caixa();
		if (caixa_bloqueado) continue;

		if (validar_codigo_aluno(codigo_aluno, senha_aluno)) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Codigo valido!");
			LCD_Escrever_Linha(1, 0, "Processando...");
			delay1ms(2000);

			int menu_ativo = 1;
			indice_menu = 0;

			while (menu_ativo) {
				verificar_status_caixa();
				if (caixa_bloqueado) break;

				LCD_limpar();
				LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
				if (indice_menu + 1 < total_opcoes)
				LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
				else
				LCD_Escrever_Linha(1, 0, " ");

				while ((tecla = varredura()) == 0) {
					verificar_status_caixa();
					if (caixa_bloqueado) break;
				}
				if (caixa_bloqueado) break;

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
						case '*':
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
