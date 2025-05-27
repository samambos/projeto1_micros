#include "utils.h"
#include "LCD.h"
#include "teclado.h"
#include "serial.h"
#include "caixa_inicial.h"
#include "timers.h"
#include <string.h>
#include <stdio.h>

extern int isBlocked(void);

extern enum Estado {
	STANDBY,
	AUTENTICACAO,
	MENU,
	OPERACAO,
	ERROR_STATE
} estado_atual;

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

void atualiza_estado(int proximo) {
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
