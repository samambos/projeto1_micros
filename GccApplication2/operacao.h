/* 
 * operacao.h - Biblioteca para operações bancárias
 * Projeto 1: Alice, Carlos e Samanta
 * Disciplina: Microcontroladores
 */

#ifndef OPERACAO_H
#define OPERACAO_H

// Definições
#define MAX_VALOR_SAQUE 1200  // Tamanho máximo para o valor do saque

// Protótipos das funções
void realizar_saque(void);
void enviar_mensagem_saque(const char* valor);
char receber_resposta_servidor(void);
void consultar_saldo(void);
void finalizar_sessao(void);

#endif /* OPERACAO_H */