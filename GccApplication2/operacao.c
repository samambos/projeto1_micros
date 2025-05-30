/* Biblioteca para operação
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#define F_CPU 16000000UL
#include <stdint.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "login.h"
#include "operacao.h"
#include "LCD.h"
#include "teclado.h"
#include "serial.h"
#include "timers.h"

uint8_t confirmar_senha(const char* senha_atual) {
	char senha_confirmacao[7] = {0};
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Confirme a senha:");
	LCD_Escrever_Linha(1, 0, "______");

	while(1) {
		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return 0;
		}

		tecla = varredura();

		if(tecla >= '0' && tecla <= '9' && pos < 6) {
			senha_confirmacao[pos] = tecla;
			LCD_Escrever_Linha(1, pos, "*");
			pos++;
			delay1ms(200);
			} else if(tecla == '#') {
			senha_confirmacao[pos] = '\0';
			
			if(strcmp(senha_confirmacao, senha_atual) == 0) {
				return 1;
				} else {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Senha incorreta!");
				LCD_Escrever_Linha(1, 0, "Tente novamente");
				delay1ms(2000);
				
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Confirme a senha:");
				LCD_Escrever_Linha(1, 0, "______");
				pos = 0;
				memset(senha_confirmacao, 0, sizeof(senha_confirmacao));
			}
			} else if(tecla == '*') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Operacao");
			LCD_Escrever_Linha(1, 0, "cancelada");
			delay1ms(2000);
			return 0;
		}
	}
}

// Pergunta sobre comprovante
void perguntar_comprovante() {
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Deseja comprovante?");
	LCD_Escrever_Linha(1, 0, "1-Sim 2-Nao");

	while(1) {
		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return;
		}

		char tecla = varredura();
		if(tecla == '1') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Comprovante em");
			LCD_Escrever_Linha(1, 0, "desenvolvimento");
			delay1ms(2000);
			return;
			} else if(tecla == '2') {
			return;
		}
	}
}

// Realiza um saque (máximo de R$1200,00)
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
			delay1ms(2000);
			return;
		}

		tecla = varredura();

		if(tecla >= '0' && tecla <= '9' && pos < (MAX_VALOR_SAQUE - 1)) {
			valor_saque[pos] = tecla;
			char str[2] = {tecla, '\0'};
			LCD_Escrever_Linha(1, 2 + pos, str);
			pos++;
			delay1ms(200);
			} else if(tecla == '#' && pos > 0) {
			valor_saque[pos] = '\0';

			// Verifica bloqueio antes de enviar a mensagem
			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			long valor_numerico = atol(valor_saque);
			if (valor_numerico > 120000L) { // Limite de R$1200,00
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Limite máximo");
				LCD_Escrever_Linha(1, 0, "R$ 1200,00");
				delay1ms(2000);
				
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Valor do saque:");
				LCD_Escrever_Linha(1, 0, "R$");
				pos = 0;
				memset(valor_saque, 0, sizeof(valor_saque));
				continue;
			}
			
			// Confirmação de senha antes de prosseguir
			if(!confirmar_senha(get_current_password())) {
				break; // Sai se a senha não for confirmada
			}
			
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
				delay1ms(2000);
				perguntar_comprovante();
				} else {
				LCD_Escrever_Linha(0, 0, "Saldo");
				LCD_Escrever_Linha(1, 0, "insuficiente");
			}
			delay1ms(3000);
			break;
			} else if(tecla == '*') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Operacao");
			LCD_Escrever_Linha(1, 0, "cancelada");
			delay1ms(2000);
			break;
		}
	}
}

// Envia mensagem de saque
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

// Recebe resposta do servidor
char receber_resposta_servidor(void) {
	char resposta[5];

	SerialRecebeChars(3, resposta);
	resposta[3] = '\0';

	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2];
	}

	return 'E';
}

// Consulta saldo
void consultar_saldo(void) {
	// Primeiro verifica a senha
	if(!confirmar_senha(get_current_password())) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Operacao");
		LCD_Escrever_Linha(1, 0, "cancelada");
		delay1ms(2000);
		return;
	}

	char mensagem[2] = { 'C', 'V' };
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "OP CANCELADA");
		LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
		delay1ms(2000);
		return;
	}
	SerialEnviaChars(2, mensagem);

	char resposta_header[3];
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "OP CANCELADA");
		LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
		delay1ms(2000);
		return;
	}
	SerialRecebeChars(3, resposta_header);
	resposta_header[3] = '\0';

	if (resposta_header[0] == 'S' && resposta_header[1] == 'V') {
		unsigned char num_bytes_saldo = resposta_header[2];

		char saldo_bruto[16];
		memset(saldo_bruto, 0, sizeof(saldo_bruto));

		if (num_bytes_saldo >= sizeof(saldo_bruto)) {
			num_bytes_saldo = sizeof(saldo_bruto) - 1;
		}

		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return;
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
		
		perguntar_comprovante();
		} else {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Erro ao obter");
		LCD_Escrever_Linha(1, 0, "saldo!");
		delay1ms(3000);
	}
}

// Finaliza a sessão
void finalizar_sessao(void) {
	char mensagem[2] = {'C', 'F'};
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "SESSAO NAO");
		LCD_Escrever_Linha(1, 0, "FINALIZADA!");
		delay1ms(2000);
		return;
	}
	SerialEnviaChars(2, mensagem);

	char resposta[3];
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "SESSAO NAO");
		LCD_Escrever_Linha(1, 0, "FINALIZADA!");
		delay1ms(2000);
		return;
	}
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