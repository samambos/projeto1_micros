/* Arquivo operacao.h para operação
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/ 
#ifndef OPERACAO_H
#define OPERACAO_H

#include <avr/io.h>
#include <string.h>

#define MAX_VALOR_SAQUE 9 // Tamanho máximo para o valor do saque (incluindo o '\0')

// Protótipos das funções
void realizar_saque(void);
void realizar_pagamento(void);
void enviar_mensagem_saque(const char* valor);
char receber_resposta_servidor(void);

#endif