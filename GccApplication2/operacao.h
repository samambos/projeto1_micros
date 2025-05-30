// GccApplication2/operacao.h
#ifndef OPERACAO_H
#define OPERACAO_H

#include <stdint.h>  // Adicionado para uint8_t

// Definições
#define MAX_VALOR_SAQUE 7  // Tamanho máximo para o valor do saque

// Protótipos das funções
void realizar_saque(void);
void enviar_mensagem_saque(const char* valor);
char receber_resposta_saque(void); // Renamed from receber_resposta_servidor to be more specific
void consultar_saldo(void);
void finalizar_sessao(void);
uint8_t confirmar_senha(const char* senha_atual);
void perguntar_comprovante();
void realizar_pagamento(void); // Added prototype for payment function
void enviar_mensagem_pagamento(const char* banco, const char* convenio, const char* valor);
char receber_resposta_pagamento(void);

#endif /* OPERACAO_H */