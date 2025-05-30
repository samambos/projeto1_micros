// GccApplication2/operacao.c
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
			LCD_Escrever_Linha(1, pos, "*"); // Display exibe * no lugar dos caracteres digitados [cite: 14]
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

// Realiza um saque (máximo de R$1200,00) [cite: 23]
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
			if (valor_numerico > 120000L) { // Limite de R$1200,00 (valor_numerico * 100 para ter 2 casas decimais)
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
			
			enviar_mensagem_saque(valor_saque); // Cada operação solicitada deve ser enviada ao aplicativo externo [cite: 12]
			
			// Verifica bloqueio antes de receber resposta
			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			char resposta = receber_resposta_saque(); // O aplicativo externo retorna se foi aceita ou não e os resultados da operação pedida [cite: 12]

			LCD_limpar();
			if(resposta == 'O') { // 'O' para OK [cite: 46]
				LCD_Escrever_Linha(0, 0, "Saque");
				LCD_Escrever_Linha(1, 0, "Realizado!");
				delay1ms(2000);
				perguntar_comprovante(); // Permite ao cliente a opção de emitir comprovante [cite: 15]
				} else if (resposta == 'I') { // 'I' para Saldo Insuficiente [cite: 46]
				LCD_Escrever_Linha(0, 0, "Saldo");
				LCD_Escrever_Linha(1, 0, "insuficiente");
				} else {
				LCD_Escrever_Linha(0, 0, "Erro na");
				LCD_Escrever_Linha(1, 0, "operacao");
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

	// A mensagem deve conter 'C', 'S', n (tamanho do valor), e o valor [cite: 46]
	// O valor é enviado como uma string de dígitos. Ex: R$50,00 é "5000" [cite: 46]
	char mensagem[3 + tamanho_valor]; // 2 bytes de cabeçalho + 1 byte de tamanho + N bytes do valor
	mensagem[0] = 'C';
	mensagem[1] = 'S';
	mensagem[2] = (char)tamanho_valor; // Número de bytes que seguem [cite: 47]
	memcpy(&mensagem[3], valor, tamanho_valor);
	SerialEnviaChars(3 + tamanho_valor, mensagem);
}

// Recebe resposta do servidor para saque
char receber_resposta_saque(void) {
	char resposta[5]; // SS + 1 byte de status (O/I) + '\0'
	SerialRecebeChars(3, resposta); // Espera 3 bytes: 'S', 'S', Status (O/I) [cite: 46]
	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2]; // Retorna 'O' (OK) ou 'I' (Saldo Insuficiente) [cite: 46]
	}
	return 'E'; // Erro
}

// Consulta saldo
void consultar_saldo(void) {
	// Primeiro verifica a senha
	if(!confirmar_senha(get_current_password())) { // Exige confirmação de senha [cite: 13]
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Operacao");
		LCD_Escrever_Linha(1, 0, "cancelada");
		delay1ms(2000);
		return;
	}

	char mensagem[2] = { 'C', 'V' }; // Mensagem para solicitar verificação de saldo [cite: 48]
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
	SerialRecebeChars(3, resposta_header); // Espera 'S', 'V', e o número de bytes do saldo [cite: 48]
	resposta_header[3] = '\0';

	if (resposta_header[0] == 'S' && resposta_header[1] == 'V') {
		unsigned char num_bytes_saldo = resposta_header[2]; // Tamanho do saldo [cite: 48]

		char saldo_bruto[16]; // Buffer para o saldo sem formatação
		memset(saldo_bruto, 0, sizeof(saldo_bruto));

		if (num_bytes_saldo >= sizeof(saldo_bruto)) {
			num_bytes_saldo = sizeof(saldo_bruto) - 1; // Previne overflow
		}

		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return;
		}
		SerialRecebeChars(num_bytes_saldo, saldo_bruto); // Recebe o saldo [cite: 48]
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
		LCD_Escrever_Linha(1, 0, saldo_formatado); // Exibe o saldo [cite: 48]
		delay1ms(3000);
		
		perguntar_comprovante(); // Permite ao cliente a opção de emitir comprovante [cite: 15]
		} else {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Erro ao obter");
		LCD_Escrever_Linha(1, 0, "saldo!");
		delay1ms(3000);
	}
}

// Realiza um pagamento
void realizar_pagamento(void) {
	char banco[4] = {0}; // 3 dígitos para o banco
	char convenio[5] = {0}; // 4 dígitos para o convênio
	char valor_pagamento[MAX_VALOR_SAQUE] = {0}; // Mesmo tamanho do saque para valor

	int pos = 0;
	char tecla;

	// Entrada do Banco
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Banco (3 dig.):");
	LCD_Escrever_Linha(1, 0, "___");
	pos = 0;
	while (pos < 3) {
		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return;
		}
		tecla = varredura();
		if (tecla >= '0' && tecla <= '9') {
			banco[pos] = tecla;
			char temp[2] = { tecla, '\0' };
			LCD_Escrever_Linha(1, pos, temp);
			pos++;
			delay1ms(200);
		} else if (tecla == '*') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Operacao");
			LCD_Escrever_Linha(1, 0, "cancelada");
			delay1ms(2000);
			return;
		}
	}
	banco[3] = '\0';

	// Entrada do Convênio
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Convenio (4 dig.):");
	LCD_Escrever_Linha(1, 0, "____");
	pos = 0;
	while (pos < 4) {
		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return;
		}
		tecla = varredura();
		if (tecla >= '0' && tecla <= '9') {
			convenio[pos] = tecla;
			char temp[2] = { tecla, '\0' };
			LCD_Escrever_Linha(1, pos, temp);
			pos++;
			delay1ms(200);
		} else if (tecla == '*') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Operacao");
			LCD_Escrever_Linha(1, 0, "cancelada");
			delay1ms(2000);
			return;
		}
	}
	convenio[4] = '\0';

	// Entrada do Valor
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Valor do pag.:");
	LCD_Escrever_Linha(1, 0, "R$");
	pos = 0;
	while(1) {
		if (isBlocked()) {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "OP CANCELADA");
			LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
			delay1ms(2000);
			return;
		}
		tecla = varredura();
		if(tecla >= '0' && tecla <= '9' && pos < (MAX_VALOR_SAQUE - 1)) {
			valor_pagamento[pos] = tecla;
			char str[2] = {tecla, '\0'};
			LCD_Escrever_Linha(1, 2 + pos, str);
			pos++;
			delay1ms(200);
		} else if(tecla == '#') {
			valor_pagamento[pos] = '\0';

			// Confirmação de senha antes de prosseguir
			if(!confirmar_senha(get_current_password())) {
				break; // Sai se a senha não for confirmada
			}
			
			enviar_mensagem_pagamento(banco, convenio, valor_pagamento); // Cada operação solicitada deve ser enviada ao aplicativo externo [cite: 12]

			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			char resposta = receber_resposta_pagamento(); // O aplicativo externo retorna se foi aceita ou não e os resultados da operação pedida [cite: 12]

			LCD_limpar();
			if(resposta == 'O') { // 'O' para OK [cite: 52]
				LCD_Escrever_Linha(0, 0, "Pagamento");
				LCD_Escrever_Linha(1, 0, "Realizado!");
				delay1ms(2000);
				perguntar_comprovante(); // Permite ao cliente a opção de emitir comprovante [cite: 15]
			} else if (resposta == 'I') { // 'I' para Saldo Insuficiente [cite: 52]
				LCD_Escrever_Linha(0, 0, "Saldo");
				LCD_Escrever_Linha(1, 0, "insuficiente");
			} else {
				LCD_Escrever_Linha(0, 0, "Erro na");
				LCD_Escrever_Linha(1, 0, "operacao");
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

// Envia mensagem de pagamento
void enviar_mensagem_pagamento(const char* banco, const char* convenio, const char* valor) {
	int tamanho_banco = strlen(banco);
	int tamanho_convenio = strlen(convenio);
	int tamanho_valor = strlen(valor);

	// Mensagem: 'C', 'P', n (total de bytes), banco, convenio, valor [cite: 52]
	int total_bytes_dados = tamanho_banco + tamanho_convenio + tamanho_valor;
	char mensagem[3 + total_bytes_dados]; // 2 bytes de cabeçalho + 1 byte de tamanho + N bytes dos dados
	
	mensagem[0] = 'C';
	mensagem[1] = 'P';
	mensagem[2] = (char)total_bytes_dados; // Número de bytes que seguem [cite: 52]

	// Copia os dados
	int current_pos = 3;
	memcpy(&mensagem[current_pos], banco, tamanho_banco);
	current_pos += tamanho_banco;
	memcpy(&mensagem[current_pos], convenio, tamanho_convenio);
	current_pos += tamanho_convenio;
	memcpy(&mensagem[current_pos], valor, tamanho_valor);

	SerialEnviaChars(3 + total_bytes_dados, mensagem);
}

// Recebe resposta do servidor para pagamento
char receber_resposta_pagamento(void) {
	char resposta[5]; // SP + 1 byte de status (O/I) + '\0'
	SerialRecebeChars(3, resposta); // Espera 3 bytes: 'S', 'P', Status (O/I) [cite: 52]
	if(resposta[0] == 'S' && resposta[1] == 'P') {
		return resposta[2]; // Retorna 'O' (OK) ou 'I' (Saldo Insuficiente) [cite: 52]
	}
	return 'E'; // Erro
}


// Finaliza a sessão
void finalizar_sessao(void) {
	char mensagem[2] = {'C', 'F'}; // Mensagem para informar que a sessão está sendo fechada [cite: 55]
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
	SerialRecebeChars(2, resposta); // Espera 'S', 'F' como resposta [cite: 55]
	resposta[2] = '\0';

	LCD_limpar();
	//LCD_Escrever_Linha(0, 0, resposta);
	//delay1ms(2000);
	
	LCD_Escrever_Linha(0, 0, "Sessao");
	LCD_Escrever_Linha(1, 0, "Finalizada!");
	
	/*if (resposta[0] == 'S' && resposta[1] == 'F') {
		LCD_Escrever_Linha(0, 0, "Sessao");
		LCD_Escrever_Linha(1, 0, "Finalizada!");
		} else {
		LCD_Escrever_Linha(0, 0, "Erro ao finalizar");
		LCD_Escrever_Linha(1, 0, "sessao!");
	}*/
	delay1ms(2000);
}