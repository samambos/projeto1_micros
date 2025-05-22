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
	// Verifica se tem 6 d�gitos
	if(strlen(codigo) == 6) {
		for(int i = 0; i < 6; i++) {
			if(codigo[i] < '0' || codigo[i] > '9') {
				return 0; // C�digo inv�lido se cont�m n�o-d�gitos
			}
		}
		return 1; // C�digo v�lido
	}
	return 0; // C�digo inv�lido
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
			LCD_Escrever_Linha(1, pos, &tecla);
			pos++;
			delay1ms(200); // Debounce
		}
	}
	codigo[6] = '\0'; // Termina a string
}

// Fun��o principal
int main(void) {
	// Inicializa��es
	prepara_teclado();
	LCD_iniciar();
	initUART(); // Inicializa comunica��o serial
	
	char codigo_aluno[7]; // 6 d�gitos + terminador nulo
	
	while(1) {
		// Mostra mensagem inicial
		mensagem_Inicial();
		
		// Aguarda at� que algo seja pressionado para come�ar
		while(varredura() == 0);
		
		// L� o c�digo do aluno
		ler_codigo_aluno(codigo_aluno);
		
		// Valida o c�digo do aluno
		if(validar_codigo_aluno(codigo_aluno)) {
			// C�digo v�lido - mostra menu de opera��es
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Codigo valido!");
			LCD_Escrever_Linha(1, 0, "Processando...");
			delay1ms(2000);
			
			// Menu de opera��es
			char opcao = 0;
			while(1) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "1-Saque 2-Deposito");
				LCD_Escrever_Linha(1, 0, "3-Pagamento *-Sair");
				
				// Aguarda sele��o de opera��o
				while(1) {
					opcao = varredura();
					if(opcao != 0) break;
					_delay_ms(100);
				}
				
				switch(opcao) {
					case '1':
					realizar_saque();
					break;
					case '2':
					// realizar_deposito();
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "Deposito");
					LCD_Escrever_Linha(1, 0, "Em desenvolvimento");
					delay1ms(2000);
					break;
					case '3':
					// realizar_pagamento();
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "Pagamento");
					LCD_Escrever_Linha(1, 0, "Em desenvolvimento");
					delay1ms(2000);
					break;
					case '*':
					// Sai do menu
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "Voltando...");
					delay1ms(1000);
					goto fim_menu; // Sai do loop do menu
					default:
					break;
				}
			}
			fim_menu:;
			
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