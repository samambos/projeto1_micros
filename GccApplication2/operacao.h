#ifndef OPERACAO_H
#define OPERACAO_H

#include <stdint.h>  // Adicionado para uint8_t

// Defini��es
#define MAX_VALOR_SAQUE 7  // Tamanho m�ximo para o valor do saque

// Prot�tipos das fun��es
void realizar_saque(void);
void enviar_mensagem_saque(const char* valor);
char receber_resposta_servidor(void);
void consultar_saldo(void);
void finalizar_sessao(void);
uint8_t confirmar_senha(const char* senha_atual);  // Corrigido o tipo de retorno
void perguntar_comprovante();

#endif /* OPERACAO_H */