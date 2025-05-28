/* Biblioteca para operação
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#include "operacao.h"
#include "LCD.h"
#include "teclado.h"
#include <util/delay.h>
#include <string.h>

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
				LCD_Escrever_Linha(0, 0, "Saque realizado!");
				LCD_Escrever_Linha(1, 0, "Retire o dinheiro");
				} else {
				LCD_Escrever_Linha(0, 0, "Saldo insuficiente");
				LCD_Escrever_Linha(1, 0, "Tente outro valor");
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

	SerialRecebeChars(4, resposta);
	resposta[4]='\0';
	
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, resposta);
	delay1ms(2000);

	if(resposta[1] == 'S' && resposta[2] == 'S') {
		return resposta[3]; // 'O' ou 'I'
	}

	return 'E'; // Erro
}

// Função para realizar um depósito
void realizar_deposito(void) {
	char valor_deposito[MAX_VALOR_SAQUE] = {0};
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Valor do deposito:");
	LCD_Escrever_Linha(1, 0, "R$");

	while(1) {
		tecla = varredura();

		if(tecla >= '0' && tecla <= '9' && pos < (MAX_VALOR_SAQUE - 1)) {
			valor_deposito[pos] = tecla;

			char str[2] = {tecla, '\0'};
			LCD_Escrever_Linha(1, 2 + pos, str);
			pos++;

			delay1ms(200);
		}
		else if(tecla == '#' && pos > 0) {
			valor_deposito[pos] = '\0';

			enviar_mensagem_deposito(valor_deposito);
			char resposta = receber_resposta_servidor();

			LCD_limpar();
			if(resposta == 'O') {
				LCD_Escrever_Linha(0, 0, "Deposito feito!");
				} else {
				LCD_Escrever_Linha(0, 0, "Erro no deposito");
			}
			delay1ms(3000);
			break;
		}
		else if(tecla == '*') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Operacao cancelada");
			delay1ms(2000);
			break;
		}
	}
}

// Função para enviar mensagem de depósito
void enviar_mensagem_deposito(const char* valor) {
	int tamanho_valor = strlen(valor);
	int tamanho_mensagem = tamanho_valor + 3;

	char mensagem[tamanho_mensagem];
	mensagem[0] = 'C';
	mensagem[1] = 'D';
	mensagem[2] = (char)tamanho_valor;

	strncpy(&mensagem[3], valor, tamanho_valor);

	SerialEnviaChars(tamanho_mensagem, mensagem);
}

// Função para consultar saldo
void consultar_saldo(void) {
	char mensagem[3] = { 'C', 'L', 0 };
	SerialEnviaChars(3, mensagem);

	char resposta[10];
	SerialRecebeChars(9, resposta);
	resposta[9] = '\0';

	LCD_limpar();
	if (resposta[0] == 'S' && resposta[1] == 'L') {
		LCD_Escrever_Linha(0, 0, "Saldo atual:");
		LCD_Escrever_Linha(1, 0, &resposta[2]);
		} else {
		LCD_Escrever_Linha(0, 0, "Erro ao obter");
		LCD_Escrever_Linha(1, 0, "saldo");
	}
	delay1ms(3000);
}
