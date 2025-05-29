/*
 * login.c
 *
 * Created: 29/05/2025 13:23:58
 *  Author: Usuario
 */ 
#include "operacao.h"
#include "LCD.h"
#include "teclado.h"
#include "serial.h"

#include <string.h>
#include <stdio.h>
#include "timers.h"
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
	LCD_Escrever_Linha(0, 4, "Aguarde...");
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