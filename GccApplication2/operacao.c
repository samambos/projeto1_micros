/* Biblioteca para opera��o
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
#include "timers.h"

/* Biblioteca para opera��o
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
#include "timers.h"

// Fun��o para realizar um saque (com limite de R$1200,00)
void realizar_saque(void) {
	char valor_saque[MAX_VALOR_SAQUE] = {0};
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Valor do saque:");
	LCD_Escrever_Linha(1, 0, "R$");

	while(1) {
		// Verifica bloqueio durante a entrada do valor
		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(200);
			return;
		}

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

			// Verifica bloqueio antes de enviar a mensagem
			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(200);
				return;
			}
			
			// Verifica��o do limite de 120000 (R$1200,00)
			if (pos >= 6) { // S� verifica se tiver 6 d�gitos (valor m�ximo R$9999,99)
				// Verifica d�gito por d�gito conforme as regras especificadas
				if (valor_saque[0] == '1') { // Primeiro d�gito � 1
					if (valor_saque[1] >= '2') { // Segundo d�gito � >= 2
						if (valor_saque[2] >= '0') { // Terceiro d�gito � >= 0
							if (valor_saque[3] >= '0') { // Quarto d�gito � >= 0
								if (valor_saque[4] >= '0') { // Quinto d�gito � >= 0
									if (valor_saque[5] >= '0') { // Sexto d�gito � >= 0
										// Valor excede 120000 (R$1200,00)
										LCD_limpar();
										LCD_Escrever_Linha(0, 0, "Limite m�ximo");
										LCD_Escrever_Linha(1, 0, "R$ 1200,00");
										delay1ms(2000);
										break;
									}
								}
							}
						}
					}
				}
			}
			
			// Se passou na verifica��o ou n�o tem 6 d�gitos, envia o saque
			enviar_mensagem_saque(valor_saque);
			
			// Verifica bloqueio antes de receber resposta
			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
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

// Fun��o para enviar mensagem de saque
void enviar_mensagem_saque(const char* valor) {
	int tamanho_valor = strlen(valor);
	int tamanho_mensagem = tamanho_valor + 3;

	char mensagem[tamanho_mensagem];
	mensagem[0] = 'C';
	mensagem[1] = 'S';
	mensagem[2] = (char)tamanho_valor; // Tamanho do valor como byte
	strncpy(&mensagem[3], valor, tamanho_valor);
	SerialEnviaChars(tamanho_mensagem, mensagem);
}

// Fun��o para receber resposta do servidor
char receber_resposta_servidor(void) {
	char resposta[5]; // Suficiente para "SSO", "SSI", "SSE" + '\0'

	// Espera 3 bytes de resposta do servidor (ex: "SSO")
	// Idealmente, SerialRecebeChars tamb�m deveria verificar isBlocked() ou ter timeout
	SerialRecebeChars(3, resposta);
	resposta[3]='\0'; // Garante termina��o nula

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, resposta); // Para debug, mostra a resposta crua
	delay1ms(2000); // Mostra a resposta por 2 segundos

	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2]; // Retorna 'O' (Ok) ou 'I' (Insuficiente)
	}

	return 'E'; // Retorna 'E' para Erro padr�o
}


// Fun��o para consultar saldo
void consultar_saldo(void) {
	char mensagem[2] = { 'C', 'V' }; // Mensagem de consulta de saldo: 'C' 'V'

	// Verifica bloqueio antes de enviar a mensagem
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "OP CANCELADA");
		LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
		delay1ms(2000);
		return;
	}
	SerialEnviaChars(2, mensagem); // Envia 2 bytes

	char resposta_header[3]; // Para 'S', 'V', 'n'
	// Verifica bloqueio antes de receber o cabe�alho da resposta
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "OP CANCELADA");
		LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
		delay1ms(2000);
		return;
	}
	// Recebe os primeiros 3 bytes da resposta (comando + tamanho do campo de dados)
	SerialRecebeChars(3, resposta_header);
	resposta_header[3] = '\0'; // Garante termina��o nula

	// Verifica se o cabe�alho da resposta � 'S' 'V'
	if (resposta_header[0] == 'S' && resposta_header[1] == 'V') {
		unsigned char num_bytes_saldo = resposta_header[2]; // 'n' � o n�mero de bytes do saldo

		char saldo_bruto[16]; // Buffer para o saldo recebido (ex: "169071")
		memset(saldo_bruto, 0, sizeof(saldo_bruto)); // Limpa o buffer

		// Limita a leitura para n�o exceder o buffer
		if (num_bytes_saldo >= sizeof(saldo_bruto)) {
			num_bytes_saldo = sizeof(saldo_bruto) - 1;
		}

		// Verifica bloqueio antes de receber os bytes do saldo
		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return;
		}
		// Recebe os 'n' bytes do saldo
		SerialRecebeChars(num_bytes_saldo, saldo_bruto);
		saldo_bruto[num_bytes_saldo] = '\0'; // Garante termina��o nula

	
		char saldo_formatado[20]; // Buffer para a string formatada (ex: "R$1690.71")
		int len_bruto = strlen(saldo_bruto);
		if (len_bruto >= 2) {
			strcpy(saldo_formatado, "R$"); // Come�a com "R$"
			// Concatena a parte dos reais (todos os d�gitos menos os �ltimos 2)
			strncat(saldo_formatado, saldo_bruto, len_bruto - 2);
			strcat(saldo_formatado, "."); // Adiciona o ponto decimal
			// Concatena os centavos (os �ltimos 2 d�gitos)
			strcat(saldo_formatado, &saldo_bruto[len_bruto - 2]);
			} else if (len_bruto == 1) { // Ex: saldo "5" -> R$0.05
			strcpy(saldo_formatado, "R$0.0");
			strcat(saldo_formatado, saldo_bruto);
			} else { // Ex: saldo "0" ou vazio -> R$0.00
			strcpy(saldo_formatado, "R$0.00");
		}


		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Saldo atual:");
		LCD_Escrever_Linha(1, 0, saldo_formatado); // Exibe o saldo formatado
		delay1ms(3000); // Exibe por 3 segundos
		} else {
		// Se a resposta do servidor n�o seguir o formato esperado
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Erro ao obter");
		LCD_Escrever_Linha(1, 0, "saldo!");
		delay1ms(3000);
	}
}

// Fun��o para finalizar a sess�o
void finalizar_sessao(void) {
	char mensagem[2] = {'C', 'F'}; // Comando para finalizar sess�o

	// Verifica bloqueio antes de enviar a mensagem
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "SESSAO NAO");
		LCD_Escrever_Linha(1, 0, "FINALIZADA!");
		delay1ms(2000);
		return;
	}
	SerialEnviaChars(2, mensagem);

	char resposta[3]; // Suficiente para "SF" + '\0'
	// Verifica bloqueio antes de receber a resposta
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "SESSAO NAO");
		LCD_Escrever_Linha(1, 0, "FINALIZADA!");
		delay1ms(2000);
		return;
	}
	SerialRecebeChars(2, resposta); // Espera 2 bytes de resposta ("SF")
	resposta[2] = '\0'; // Garante termina��o nula

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