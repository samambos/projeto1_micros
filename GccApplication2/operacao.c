/* Biblioteca para operação
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#include "operacao.h"
#include "LCD.h"
#include "teclado.h"
#include <util/delay.h>

// Função para realizar um saque
void realizar_saque(void) {
	char valor_saque[MAX_VALOR_SAQUE] = {0};
	int pos = 0;
	char tecla;
	
	// Solicita o valor do saque ao usuário
	LCD_limpar();
	LCD_Escrever_Linha(0, 0, "Valor do saque:");
	LCD_Escrever_Linha(1, 0, "R$");
	
	while(1) {
		tecla = varredura();
		
		// Se for dígito e ainda houver espaço
		if(tecla >= '0' && tecla <= '9' && pos < (MAX_VALOR_SAQUE-1)) {
			valor_saque[pos] = tecla;
			LCD_Escrever_Linha(1, 2+pos, &tecla);
			pos++;
			delay1ms(200); // Debounce
		}
		// Tecla enter (confirmar)
		else if(tecla == '#' && pos > 0) {
			valor_saque[pos] = '\0'; // Finaliza a string
			
			// Envia a mensagem de saque para o servidor
			enviar_mensagem_saque(valor_saque);
			
			// Aguarda resposta do servidor
			char resposta = receber_resposta_servidor();
			
			LCD_limpar();
			if(resposta == 'O') { // OK
				LCD_Escrever_Linha(0, 0, "Saque realizado!");
				LCD_Escrever_Linha(1, 0, "Retire o dinheiro");
				} else { // Saldo Insuficiente
				LCD_Escrever_Linha(0, 0, "Saldo insuficiente");
				LCD_Escrever_Linha(1, 0, "Tente outro valor");
			}
			_delay_ms(3000);
			break;
		}
		// Tecla cancelar
		else if(tecla == '*') {
			LCD_limpar();
			LCD_Escrever_Linha(0, 0, "Operacao");
			LCD_Escrever_Linha(1, 0, "cancelada");
			_delay_ms(2000);
			break;
		}
	}
}

// Função para enviar mensagem de saque para o servidor
void enviar_mensagem_saque(const char* valor) {
	int tamanho_valor = strlen(valor);
	int tamanho_mensagem = tamanho_valor + 3; // 'C' + 'S' + n + valor
	
	char mensagem[tamanho_mensagem];
	
	// Monta a mensagem conforme o protocolo
	mensagem[0] = 'C'; // Comando
	mensagem[1] = 'S'; // Subcomando (Saque)
	mensagem[2] = (char)tamanho_valor; // Tamanho do valor
	
	// Copia o valor para a mensagem
	strncpy(&mensagem[3], valor, tamanho_valor);
	
	// Envia a mensagem via serial
	SerialEnviaChars(tamanho_mensagem, mensagem);
}

// Função para receber resposta do servidor
char receber_resposta_servidor(void) {
	char resposta[3];
	
	// Aguarda a resposta do servidor (3 bytes)
	SerialRecebeChars(3, resposta);
	
	// Verifica se é uma resposta de saque
	if(resposta[0] == 'S' && resposta[1] == 'S') {
		return resposta[2]; // Retorna 'O' ou 'I'
	}
	
	return 'E'; // Erro
}