/* Biblioteca para operação
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#include "operacao.h"
#include "LCD.h"
#include "teclado.h"
#include "serial.h" // Adicionado para SerialEnviaChars e SerialRecebeChars
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
	char resposta[5]; // Suficiente para "SSO", "SSI", "SSE" + '\0'

	// Espera 3 bytes de resposta do servidor (ex: "SSO")
	SerialRecebeChars(3, resposta);
	resposta[3]='\0'; // Garante terminação nula

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, resposta);
	delay1ms(2000);

	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2]; // Retorna 'O' (Ok) ou 'I' (Insuficiente)
	}

	return 'E'; // Retorna 'E' para Erro padrão
}


// Função para consultar saldo
void consultar_saldo(void) {
	char mensagem[3] = { 'C', 'L', 0 }; // 'CL' + 1 byte de lixo (0)
	SerialEnviaChars(3, mensagem);

	char resposta[10]; // Suficiente para "SL" + 6 dígitos de saldo + '\0' (total 9 bytes)
	// Espera 8 bytes de resposta do servidor (ex: "SL123456")
	SerialRecebeChars(8, resposta);
	resposta[8] = '\0'; // Garante terminação nula

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


// Nova função para finalizar a sessão
void finalizar_sessao(void) {
	char mensagem[2] = {'C', 'F'}; // Comando para finalizar sessão
	SerialEnviaChars(2, mensagem);

	char resposta[3]; // Suficiente para "SF" + '\0'
	SerialRecebeChars(2, resposta); // Espera 2 bytes de resposta ("SF")
	resposta[2] = '\0'; // Garante terminação nula

	LCD_limpar();
	
	if (resposta[0] == 'S' && resposta[1] == 'F') {
		LCD_Escrever_Linha(0, 0, "Sessao");
		LCD_Escrever_Linha(1, 0, "Finalizada!");
		} else {
		LCD_Escrever_Linha(0, 0, "Erro ao finalizar");
		LCD_Escrever_Linha(1, 0, "sessao!");
	}
	delay1ms(2000); // Exibe a mensagem por 2 segundos
}