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
			LCD_Escrever_Linha(1, pos, "*"); // Display exibe * no lugar dos caracteres digitados [cite: 230]
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

// Pergunta sobre comprovante [cite: 231]
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

// Realiza um saque (máximo de R$1200,00) [cite: 239]
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
			
			// Confirmação de senha antes de prosseguir [cite: 229]
			if(!confirmar_senha(get_current_password())) {
				break; // Sai se a senha não for confirmada
			}
			
			enviar_mensagem_saque(valor_saque); // Cada operação solicitada deve ser enviada ao aplicativo externo [cite: 228]
			
			// Verifica bloqueio antes de receber resposta
			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			char resposta = receber_resposta_saque(); // O aplicativo externo retorna se foi aceita ou não e os resultados da operação pedida [cite: 228]

			LCD_limpar();
			if(resposta == 'O') { // 'O' para OK [cite: 262]
				LCD_Escrever_Linha(0, 0, "Saque");
				LCD_Escrever_Linha(1, 0, "Realizado!");
				delay1ms(2000);
				perguntar_comprovante(); // Permite ao cliente a opção de emitir comprovante [cite: 231]
				} else if (resposta == 'I') { // 'I' para Saldo Insuficiente [cite: 262]
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

// Envia mensagem de saque [cite: 262]
void enviar_mensagem_saque(const char* valor) {
	int tamanho_valor = strlen(valor);

	// A mensagem deve conter 'C', 'S', n (tamanho do valor), e o valor
	// O valor é enviado como uma string de dígitos. Ex: R$50,00 é "5000"
	char mensagem[3 + tamanho_valor]; // 2 bytes de cabeçalho + 1 byte de tamanho + N bytes do valor
	mensagem[0] = 'C';
	mensagem[1] = 'S';
	mensagem[2] = (char)tamanho_valor; // Número de bytes que seguem
	memcpy(&mensagem[3], valor, tamanho_valor);
	SerialEnviaChars(3 + tamanho_valor, mensagem);
}

// Recebe resposta do servidor para saque [cite: 262]
char receber_resposta_saque(void) {
	char resposta[5]; // SS + 1 byte de status (O/I) + '\0'
	SerialRecebeChars(3, resposta); // Espera 3 bytes: 'S', 'S', Status (O/I)
	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2]; // Retorna 'O' (OK) ou 'I' (Saldo Insuficiente)
	}
	return 'E'; // Erro
}

// Consulta saldo [cite: 239]
void consultar_saldo(void) {
	// Primeiro verifica a senha
	if(!confirmar_senha(get_current_password())) { // Exige confirmação de senha [cite: 229]
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Operacao");
		LCD_Escrever_Linha(1, 0, "cancelada");
		delay1ms(2000);
		return;
	}

	char mensagem[2] = { 'C', 'V' }; // Mensagem para solicitar verificação de saldo [cite: 264]
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
	SerialRecebeChars(3, resposta_header); // Espera 'S', 'V', e o número de bytes do saldo [cite: 264]
	resposta_header[3] = '\0';

	if (resposta_header[0] == 'S' && resposta_header[1] == 'V') {
		unsigned char num_bytes_saldo = resposta_header[2]; // Tamanho do saldo [cite: 264]

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
		SerialRecebeChars(num_bytes_saldo, saldo_bruto); // Recebe o saldo [cite: 264]
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
		LCD_Escrever_Linha(1, 0, saldo_formatado); // Exibe o saldo
		delay1ms(3000);
		
		perguntar_comprovante(); // Permite ao cliente a opção de emitir comprovante [cite: 231]
		} else {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Erro ao obter");
		LCD_Escrever_Linha(1, 0, "saldo!");
		delay1ms(3000);
	}
}

// Realiza um pagamento manualmente (existing functionality, renamed) [cite: 239]
void realizar_pagamento_manual(void) {
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

			// Confirmação de senha antes de prosseguir [cite: 229]
			if(!confirmar_senha(get_current_password())) {
				break; // Sai se a senha não for confirmada
			}
			
			enviar_mensagem_pagamento(banco, convenio, valor_pagamento); // Cada operação solicitada deve ser enviada ao aplicativo externo [cite: 228]

			if (isBlocked()) {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "OP CANCELADA");
				LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
				delay1ms(2000);
				return;
			}
			
			char resposta = receber_resposta_pagamento(); // O aplicativo externo retorna se foi aceita ou não e os resultados da operação pedida [cite: 228]

			LCD_limpar();
			if(resposta == 'O') { // 'O' para OK [cite: 269]
				LCD_Escrever_Linha(0, 0, "Pagamento");
				LCD_Escrever_Linha(1, 0, "Realizado!");
				delay1ms(2000);
				perguntar_comprovante(); // Permite ao cliente a opção de emitir comprovante [cite: 231]
			} else if (resposta == 'I') { // 'I' para Saldo Insuficiente [cite: 269]
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

// Realiza pagamento por código de barras [cite: 240]
void realizar_pagamento_barcode(void) {
    char barcode_raw[100]; // Assume a max barcode length
    char banco[4], convenio[5], valor_pagamento[MAX_VALOR_SAQUE];
    char c;
    int received_bytes = 0;
	int timeout_count = 0;
	const int timeout_limit = 500; // 5 seconds timeout

    LCD_limpar();
    LCD_Escrever_Linha(0, 0, "Aguardando barcode");
    LCD_Escrever_Linha(1, 0, "do servidor...");

    memset(barcode_raw, 0, sizeof(barcode_raw));
    memset(banco, 0, sizeof(banco));
    memset(convenio, 0, sizeof(convenio));
    memset(valor_pagamento, 0, sizeof(valor_pagamento));

    // Wait for the 'S' 'B' message from the server [cite: 265]
	char header[3];
	SerialRecebeChars(2, header); // Expect 'S', 'B'
	header[2] = '\0';

	if (header[0] == 'S' && header[1] == 'B') {
		unsigned char num_bytes_barcode = 0;
		SerialRecebeChars(1, &num_bytes_barcode); // Get the length of the barcode data [cite: 265]

		if (num_bytes_barcode > 0 && num_bytes_barcode < sizeof(barcode_raw)) {
			// Receive the barcode data
			SerialRecebeChars(num_bytes_barcode, barcode_raw); // Receives barcode data as a vector [cite: 242]
			barcode_raw[num_bytes_barcode] = '\0';
            
            // Interpret the barcode data [cite: 242]
			if (parse_barcode_data(barcode_raw, banco, convenio, valor_pagamento)) {
				// Display parsed data for confirmation (optional, for debugging)
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Bco:");
				LCD_Escrever_Linha(0, 5, banco);
				LCD_Escrever_Linha(1, 0, "Val: R$");
				LCD_Escrever_Linha(1, 7, valor_pagamento);
				delay1ms(3000); // Show for a few seconds

				// Confirmation of senha before proceeding [cite: 229]
				if(!confirmar_senha(get_current_password())) {
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "Operacao");
					LCD_Escrever_Linha(1, 0, "cancelada");
					delay1ms(2000);
					return;
				}

				enviar_mensagem_pagamento(banco, convenio, valor_pagamento); // Send to server [cite: 228]

				if (isBlocked()) {
					LCD_limpar();
					LCD_Escrever_Linha(0, 0, "OP CANCELADA");
					LCD_Escrever_Linha(1, 0, "SISTEMA BLOQ.");
					delay1ms(2000);
					return;
				}

				char resposta = receber_resposta_pagamento(); // Get response from server [cite: 228]

				LCD_limpar();
				if(resposta == 'O') {
					LCD_Escrever_Linha(0, 0, "Pagamento");
					LCD_Escrever_Linha(1, 0, "Realizado!");
					delay1ms(2000);
					perguntar_comprovante(); // [cite: 231]
				} else if (resposta == 'I') {
					LCD_Escrever_Linha(0, 0, "Saldo");
					LCD_Escrever_Linha(1, 0, "insuficiente");
				} else {
					LCD_Escrever_Linha(0, 0, "Erro no pag.");
				}
				delay1ms(3000);
			} else {
				LCD_limpar();
				LCD_Escrever_Linha(0, 0, "Barcode Invalido!");
				LCD_Escrever_Linha(1, 0, "Tente novamente");
				delay1ms(3000);
			}
		} else {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Erro ao receber");
			LCD_Escrever_Linha(1, 0, "barcode!");
			delay1ms(3000);
		}
	} else {
		LCD_limpar();
		LCD_Escrever_Linha(0, 0, "Nenhum barcode");
		LCD_Escrever_Linha(1, 0, "recebido.");
		delay1ms(3000);
	}
}

// Envia mensagem de pagamento [cite: 268]
void enviar_mensagem_pagamento(const char* banco, const char* convenio, const char* valor) {
	int tamanho_banco = strlen(banco);
	int tamanho_convenio = strlen(convenio);
	int tamanho_valor = strlen(valor);

	// Mensagem: 'C', 'P', n (total de bytes), banco, convenio, valor [cite: 268]
	int total_bytes_dados = tamanho_banco + tamanho_convenio + tamanho_valor;
	char mensagem[3 + total_bytes_dados]; // 2 bytes de cabeçalho + 1 byte de tamanho + N bytes dos dados
	
	mensagem[0] = 'C';
	mensagem[1] = 'P';
	mensagem[2] = (char)total_bytes_dados; // Número de bytes que seguem

	// Copia os dados
	int current_pos = 3;
	memcpy(&mensagem[current_pos], banco, tamanho_banco);
	current_pos += tamanho_banco;
	memcpy(&mensagem[current_pos], convenio, tamanho_convenio);
	current_pos += tamanho_convenio;
	memcpy(&mensagem[current_pos], valor, tamanho_valor);

	SerialEnviaChars(3 + total_bytes_dados, mensagem);
}

// Recebe resposta do servidor para pagamento [cite: 268]
char receber_resposta_pagamento(void) {
	char resposta[5]; // SP + 1 byte de status (O/I) + '\0'
	SerialRecebeChars(3, resposta); // Espera 3 bytes: 'S', 'P', Status (O/I)
	if(resposta[0] == 'S' && resposta[1] == 'P') {
		return resposta[2]; // Retorna 'O' (OK) ou 'I' (Saldo Insuficiente)
	}
	return 'E'; // Erro
}

// Função para calcular o DV de um campo (Módulo 10) [cite: 145]
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

// Função para calcular o DV do código de barras (Módulo 11) [cite: 163]
int calculate_module11_dv(const char* data, int length) {
    int sum = 0;
    int multiplier = 2;
    for (int i = length - 1; i >= 0; i--) {
        // Skip the DV position (5th position in a 44-char barcode, which is index 4 for 0-indexed string)
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

    if (dv == 0 || dv == 10 || dv == 11) { // DV equal to 0, 10 or 11 should be 1 [cite: 170]
        return 1;
    } else {
        return dv;
    }
}


// Função para extrair dados de um código de barras (44 posições) [cite: 38]
char parse_barcode_data(const char* barcode_data, char* banco, char* convenio, char* valor) {
    // Barcode structure (from Doc5175Bloqueto.pdf, section 2.3.2 and 2.3.4, Anexos VII, VIII, IX)
    // 01 a 03: Código do Banco [cite: 39]
    // 04 a 04: Código da Moeda (9-Real) [cite: 39]
    // 05 a 05: Dígito Verificador (DV) do código de Barras [cite: 39] (Calculated using Module 11) [cite: 163]
    // 06 a 09: Fator de Vencimento [cite: 39]
    // 10 a 19: Valor (8 inteiros, 2 decimais) [cite: 39]
    // 20 a 44: Campo Livre [cite: 39] (This is where bank-specific info like Nosso Número, Agency, Account, etc. are)

    // Check if barcode_data has expected length (44 digits)
    if (strlen(barcode_data) != 44) {
        return 0; // Invalid barcode length
    }

    // Extract Banco (3 digits from pos 1-3)
    strncpy(banco, barcode_data, 3);
    banco[3] = '\0';

    // Extract Valor (10 digits from pos 10-19)
    char temp_valor[11];
    strncpy(temp_valor, barcode_data + 9, 10);
    temp_valor[10] = '\0';
    // The payment amount for CP message is in cents, so the 2 decimals are included in the string.
    strcpy(valor, temp_valor);


    // For "Convenio", we need to figure out which type of convenio it is (4, 6, or 7 positions)
    // This example will use the 'Nosso Número' field from the 'Campo Livre' (pos 20-44)
    // Assuming a 4-position convenio for this example as per the CP message.
    // In a real scenario, you would need to parse the "Campo Livre" (positions 20-44)
    // based on the specific convenio format (e.g., Anexo VII, VIII, IX) to get the correct convenio number.
    // For simplicity, let's assume the convenio is the first 4 digits of the 'Nosso Número' (pos 20-23) if using Anexo VII example. [cite: 180]
    strncpy(convenio, barcode_data + 19, 4); // Nosso Número starts at pos 20 (index 19) [cite: 180]
    convenio[4] = '\0';
    
    // Perform verification checks (e.g., Module 10 for fields, Module 11 for barcode DV) [cite: 145, 163]
    // This is a simplified example; a full implementation would involve complex DV calculations
    // for each field of the 'linha digitavel' and the overall barcode.
    
    // Basic validation of the barcode's main DV (5th position) [cite: 39]
    char barcode_dv_char = barcode_data[4];
    char barcode_data_without_dv[44];
    strncpy(barcode_data_without_dv, barcode_data, 4);
    strncpy(barcode_data_without_dv + 4, barcode_data + 5, 39);
    barcode_data_without_dv[43] = '\0';

    int expected_dv = calculate_module11_dv(barcode_data, 44); // Use the full barcode for DV calculation
    if ((barcode_dv_char - '0') != expected_dv) {
        return 0; // Barcode DV mismatch
    }

    // You would also perform checks for the "linha digitavel" fields' DVs here using calculate_module10_dv [cite: 45]
    // Example (simplified):
    // char campo1_str[10]; // AAABC.CCCCX [cite: 47]
    // strncpy(campo1_str, barcode_data, 3); // AAA
    // strncpy(campo1_str + 3, barcode_data + 4, 1); // B
    // strncpy(campo1_str + 4, barcode_data + 20, 5); // CCCCC from "Campo Livre"
    // campo1_str[9] = '\0';
    // int campo1_calculated_dv = calculate_module10_dv(campo1_str, 9);
    // if (campo1_calculated_dv != (barcode_data[9] - '0')) return 0; // Check DV X

    return 1; // Barcode parsed and basic validation passed
}


// Finaliza a sessão [cite: 271]
void finalizar_sessao(void) {
	char mensagem[2] = {'C', 'F'}; // Mensagem para informar que a sessão está sendo fechada [cite: 271]
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
	SerialRecebeChars(2, resposta); // Espera 'S', 'F' como resposta [cite: 271]
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