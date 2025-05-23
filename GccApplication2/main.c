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

// Fun��o para validar o c�digo do aluno
int validar_codigo_aluno(const char* codigo) {
	// Verifica se tem 6 d�gitos num�ricos
	if (strlen(codigo) != 6) return 0;
	for (int i = 0; i < 6; i++) {
		if (codigo[i] < '0' || codigo[i] > '9') return 0;
	}

	// Monta a mensagem no formato 'C' 'E' + c�digo
	char mensagem[8];
	mensagem[0] = 'C';
	mensagem[1] = 'E';
	memcpy(&mensagem[2], codigo, 6);

	// Envia a mensagem ao servidor
	SerialEnviaChars(8, mensagem);

	// Aguarda resposta do servidor
	char resposta[32];
	SerialRecebeChars(18, resposta); // Tamanho m�ximo esperado

	// Verifica se a resposta come�a com 'S' 'E'
	if (resposta[0] == 'S' && resposta[1] == 'E') {
		if (strstr(resposta, "Nao Autorizado") != NULL) {
			return 0; // C�digo inv�lido
			} else {
			return 1; // C�digo v�lido
		}
	}

	return 0; // Falha na comunica��o ou resposta inesperada
}


// Fun��o para ler o c�digo do aluno do teclado
void ler_codigo_aluno(char* codigo) {
	int pos = 0;
	char tecla;
	
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Digite codigo:");
	LCD_Escrever_Linha(1, 0, "______"); // 6 underscores
	
	while(pos < 6) {
		tecla = varredura();
		if(tecla >= '0' && tecla <= '9') {
			codigo[pos] = tecla;
			char temp[2] = {tecla, '\0'};
			LCD_Escrever_Linha(1, pos, temp);
			pos++;
			delay1ms(200); // Debounce
		}
	}
	codigo[6] = '\0'; // Termina a string
}


int main(void) {
	// Inicializa��es
	prepara_teclado();
	LCD_iniciar();
	initUART(); // Inicializa comunica��o serial

	char codigo_aluno[7]; // 6 d�gitos + terminador nulo

	const char* opcoes[] = {
		"1-Saque",
		"2-Deposito",
		"3-Pagamento",
		"*-Saldo"
	};
	const int total_opcoes = 4;
	int indice_menu = 0;
	char tecla;

	while (1) {
		// Mostra mensagem inicial
		mensagem_Inicial();

		// Aguarda at� que algo seja pressionado para come�ar
		while (varredura() == 0);

		// L� o c�digo do aluno
		ler_codigo_aluno(codigo_aluno);

		// Valida o c�digo do aluno via UART
		if (1<2) {														 // if(validar_codigo_aluno(codigo_aluno)) arrumar para vers�o final!!! isso pula a verifica��o do servidor
			// C�digo v�lido - mostra mensagem de sucesso
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Codigo valido!");
			LCD_Escrever_Linha(1, 0, "Processando...");
			delay1ms(2000);

			// Menu com rolagem
			int menu_ativo = 1;
			indice_menu = 0;

			while (menu_ativo) {
				// Exibe duas op��es por vez
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, opcoes[indice_menu]);
				if (indice_menu + 1 < total_opcoes) {
					LCD_Escrever_Linha(1, 0, opcoes[indice_menu + 1]);
					} else {
					LCD_Escrever_Linha(1, 0, " ");
				}

				// Aguarda tecla
				while ((tecla = varredura()) == 0);
				delay1ms(300); // debounce

				if (tecla == 'B') {
					if (indice_menu < total_opcoes - 2) {
						indice_menu++;
					}
					} else if (tecla == 'A') {
					if (indice_menu > 0) {
						indice_menu--;
					}
					} else if (tecla == '*') {
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "Voltando...");
					delay1ms(1000);
					menu_ativo = 0; // Sai do menu
					} else if (tecla == opcoes[indice_menu][0]) {
					// Executa a opera��o correspondente � tecla pressionada
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
			// C�digo inv�lido - mostra mensagem e volta ao in�cio
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Conta invalida!");
			LCD_Escrever_Linha(1, 0, "Tente novamente");
			delay1ms(2000);
		}
	}

	return 0;
}
