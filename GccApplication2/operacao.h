/* Arquivo operacao.h para opera��o
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/ 
#ifndef OPERACAO_H
#define OPERACAO_H

#include <avr/io.h>
#include <string.h>

#define MAX_VALOR_SAQUE 9 // Tamanho m�ximo para o valor do saque (incluindo o '\0')

// Prot�tipos das fun��es
void realizar_saque(void);
void realizar_pagamento(void);
void enviar_mensagem_saque(const char* valor);
char receber_resposta_servidor(void);

#endif