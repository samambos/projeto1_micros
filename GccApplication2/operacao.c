// GccApplication2/operacao.c
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

void realizar_saque(void) {
	char valor_saque[MAX_VALOR_SAQUE] = {0};
	int pos = 0;
	char tecla;

	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Valor do saque:");
	LCD_Escrever_Linha(1, 0, "R$");

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
			valor_saque[pos] = tecla;
			char str[2] = {tecla, '\0'};
			LCD_Escrever_Linha(1, 2 + pos, str);
			pos++;
			delay1ms(200);
			} else if(tecla == '#' && pos > 0) {
			valor_saque[pos] = '\0';

			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			long valor_numerico = atol(valor_saque);
			if (valor_numerico > 120000L) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Limite maximo");
				LCD_Escrever_Linha(1, 0, "R$ 1200,00");
				delay1ms(2000);
				
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Valor do saque:");
				LCD_Escrever_Linha(1, 0, "R$");
				pos = 0;
				memset(valor_saque, 0, sizeof(valor_saque));
				continue;
			}
			
			if(!confirmar_senha(get_current_password())) {
				break;
			}
			
			enviar_mensagem_saque(valor_saque);
			
			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			char resposta = receber_resposta_saque();

			LCD_limpar();
			if(resposta == 'O') {
				LCD_Escrever_Linha(0, 0, "Saque");
				LCD_Escrever_Linha(1, 0, "Realizado!");
				delay1ms(2000);
				perguntar_comprovante();
				} else if (resposta == 'I') {
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

void enviar_mensagem_saque(const char* valor) {
	int tamanho_valor = strlen(valor);
	char mensagem[3 + tamanho_valor];
	mensagem[0] = 'C';
	mensagem[1] = 'S';
	mensagem[2] = (char)tamanho_valor;
	memcpy(&mensagem[3], valor, tamanho_valor);
	SerialEnviaChars(3 + tamanho_valor, mensagem);
}

char receber_resposta_saque(void) {
	char resposta[5];
	SerialRecebeChars(3, resposta);
	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2];
	}
	return 'E';
}

void consultar_saldo(void) {
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

void realizar_pagamento_manual(void) {
	char banco[4] = {0};
	char convenio[5] = {0};
	char valor_pagamento[MAX_VALOR_SAQUE] = {0};

	int pos = 0;
	char tecla;

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

			if(!confirmar_senha(get_current_password())) {
				break;
			}
			
			enviar_mensagem_pagamento(banco, convenio, valor_pagamento);

			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			char resposta = receber_resposta_pagamento();

			LCD_limpar();
			if(resposta == 'O') {
				LCD_Escrever_Linha(0, 0, "Pagamento");
				LCD_Escrever_Linha(1, 0, "Realizado!");
				delay1ms(2000);
				perguntar_comprovante();
				} else if (resposta == 'I') {
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

// Função para pagamento por código de barras (em desenvolvimento)
void realizar_pagamento_barcode(void) {
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Barcode em");
	LCD_Escrever_Linha(1, 0, "desenvolvimento");
	delay1ms(2000);
	
	if (isBlocked()) {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "OP CANCELADA");
		LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
		delay1ms(2000);
	}
}


void enviar_mensagem_pagamento(const char* banco, const char* convenio, const char* valor) {
	int tamanho_banco = strlen(banco);
	int tamanho_convenio = strlen(convenio);
	int tamanho_valor = strlen(valor);

	int total_bytes_dados = tamanho_banco + tamanho_convenio + tamanho_valor;
	char mensagem[3 + total_bytes_dados];
	
	mensagem[0] = 'C';
	mensagem[1] = 'P';
	mensagem[2] = (char)total_bytes_dados;

	int current_pos = 3;
	memcpy(&mensagem[current_pos], banco, tamanho_banco);
	current_pos += tamanho_banco;
	memcpy(&mensagem[current_pos], convenio, tamanho_convenio);
	current_pos += tamanho_convenio;
	memcpy(&mensagem[current_pos], valor, tamanho_valor);

	SerialEnviaChars(3 + total_bytes_dados, mensagem);
}

char receber_resposta_pagamento(void) {
	char resposta[5];
	SerialRecebeChars(3, resposta);
	if(resposta[0] == 'S' && resposta[1] == 'P') {
		return resposta[2];
	}
	return 'E';
}

int calculate_module10_dv(const char* data, int length) {
	int sum = 0;
	int multiplier = 2;
	for (int i = length - 1; i >= 0; i--) {
		int digit = data[i] - '0';
		int product = digit * multiplier;
		if (product > 9) {
			product = (product % 10) + (product / 10);
		}
		sum += product;
		multiplier = (multiplier == 2) ? 1 : 2;
	}
	int remainder = sum % 10;
	if (remainder == 0) {
		return 0;
		} else {
		return 10 - remainder;
	}
}

int calculate_module11_dv(const char* data, int length) {
	int sum = 0;
	int multiplier = 2;
	for (int i = length - 1; i >= 0; i--) {
		if (i == 4) continue;
		int digit = data[i] - '0';
		sum += digit * multiplier;
		multiplier++;
		if (multiplier > 9) {
			multiplier = 2;
		}
	}
	int remainder = sum % 11;
	int dv = 11 - remainder;

	if (dv == 0 || dv == 10 || dv == 11) {
		return 1;
		} else {
		return dv;
	}
}

char parse_barcode_data(const char* barcode_data, char* banco, char* convenio, char* valor) {
	if (strlen(barcode_data) != 44) {
		return 0;
	}

	strncpy(banco, barcode_data, 3);
	banco[3] = '\0';

	char temp_valor[11];
	strncpy(temp_valor, barcode_data + 9, 10);
	temp_valor[10] = '\0';
	strcpy(valor, temp_valor);

	strncpy(convenio, barcode_data + 19, 4);
	convenio[4] = '\0';
	
	char barcode_dv_char = barcode_data[4];
	char barcode_data_without_dv[44];
	strncpy(barcode_data_without_dv, barcode_data, 4);
	strncpy(barcode_data_without_dv + 4, barcode_data + 5, 39);
	barcode_data_without_dv[43] = '\0';

	int expected_dv = calculate_module11_dv(barcode_data, 44);
	if ((barcode_dv_char - '0') != expected_dv) {
		return 0;
	}

	return 1;
}

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
	
	LCD_Escrever_Linha(0, 0, "Sessao");
	LCD_Escrever_Linha(1, 0, "Finalizada!");
	
	delay1ms(2000);
}