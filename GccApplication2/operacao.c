/* Biblioteca para operação
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#include "operacao.h"
#include "LCD.h"
#include "teclado.h"
#include "serial.h"
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

// Função para realizar um saque
void realizar_saque(void) {
	char valor_saque[MAX_VALOR_SAQUE] = {0};
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Valor do saque:");
	LCD_Escrever_Linha(1, 0, "R$");

	while(1) {
		tecla = varredura();

		if(tecla >= '0' && tecla <= '9' && pos < (MAX_VALOR_SAQUE - 1)) {
			valor_saque[pos] = tecla;

			char str[2] = {tecla, '\0'};
			LCD_Escrever_Linha(1, 2 + pos, str);
			pos++;

			delay1ms(200);
		}
		else if(tecla == '#' && pos > 0) {
			valor_saque[pos] = '\0';

			enviar_mensagem_saque(valor_saque);
			char resposta = receber_resposta_servidor();

			LCD_limpar();
			if(resposta == 'O') {
				LCD_Escrever_Linha(0, 0, "Saque");
				LCD_Escrever_Linha(1, 0, "Realizado!");
				} else {
				LCD_Escrever_Linha(0, 0, "Saldo");
				LCD_Escrever_Linha(1, 0, "insuficiente");
			}
			delay1ms(3000);
			break;
		}
		else if(tecla == '*') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Operacao");
			LCD_Escrever_Linha(1, 0, "cancelada");
			delay1ms(2000);
			break;
		}
	}
}

// Função para enviar mensagem de saque
void enviar_mensagem_saque(const char* valor) {
	int tamanho_valor = strlen(valor);
	int tamanho_mensagem = tamanho_valor + 3;

	char mensagem[tamanho_mensagem];
	mensagem[0] = 'C';
	mensagem[1] = 'S';
	mensagem[2] = (char)tamanho_valor;

	strncpy(&mensagem[3], valor, tamanho_valor);

	SerialEnviaChars(tamanho_mensagem, mensagem);
}

// Função para receber resposta do servidor
char receber_resposta_servidor(void) {
	char resposta[5];

	SerialRecebeChars(3, resposta);
	resposta[3]='\0';

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, resposta);
	delay1ms(2000);

	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2];
	}

	return 'E';
}


// Função para consultar saldo
void consultar_saldo(void) {
	char mensagem[2] = { 'C', 'V' };
	SerialEnviaChars(2, mensagem);

	char resposta_header[3];
	SerialRecebeChars(3, resposta_header);
	resposta_header[3] = '\0';

	if (resposta_header[0] == 'S' && resposta_header[1] == 'V') {
		unsigned char num_bytes_saldo = resposta_header[2];

		char saldo_bruto[16];
		memset(saldo_bruto, 0, sizeof(saldo_bruto));

		if (num_bytes_saldo >= sizeof(saldo_bruto)) {
			num_bytes_saldo = sizeof(saldo_bruto) - 1;
		}

		SerialRecebeChars(num_bytes_saldo, saldo_bruto);
		saldo_bruto[num_bytes_saldo] = '\0';

		char saldo_formatado[20];
		int len_bruto = strlen(saldo_bruto);

		if (len_bruto >= 2) {
			strcpy(saldo_formatado, "R$");
			strncat(saldo_formatado, saldo_bruto, len_bruto - 2);
			strcat(saldo_formatado, ".");
			strcat(saldo_formatado, &saldo_bruto[len_bruto - 2]);
			} else if (len_bruto == 1) {
			strcpy(saldo_formatado, "R$0.0");
			strcat(saldo_formatado, saldo_bruto);
			} else {
			strcpy(saldo_formatado, "R$0.00");
		}

		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Saldo atual:");
		LCD_Escrever_Linha(1, 0, saldo_formatado);
		delay1ms(3000);
		} else {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Erro na resposta");
		LCD_Escrever_Linha(1, 0, "do servidor!");
		delay1ms(3000);
	}
}

// Função para finalizar a sessão
void finalizar_sessao(void) {
	char mensagem[2] = {'C', 'F'};
	SerialEnviaChars(2, mensagem);

	char resposta[3];
	SerialRecebeChars(2, resposta);
	resposta[2] = '\0';

	LCD_limpar();
	
	if (resposta[0] == 'S' && resposta[1] == 'F') {
		LCD_Escrever_Linha(0, 0, "Sessao");
		LCD_Escrever_Linha(1, 0, "Finalizada!");
		} else {
		LCD_Escrever_Linha(0, 0, "Erro ao finalizar");
		LCD_Escrever_Linha(1, 0, "sessao!");
	}
	delay1ms(2000);
}