// GccApplication2/operacao.h
#ifndef OPERACAO_H
#define OPERACAO_H

#include <stdint.h>  // Adicionado para uint8_t

// Definições
#define MAX_VALOR_SAQUE 7  // Tamanho máximo para o valor do saque

// Protótipos das funções
void realizar_saque(void);
void enviar_mensagem_saque(const char* valor);
char receber_resposta_saque(void);
void consultar_saldo(void);
void finalizar_sessao(void);
uint8_t confirmar_senha(const char* senha_atual);
void perguntar_comprovante();

void realizar_pagamento_manual(void); // Renamed from realizar_pagamento
void realizar_pagamento_barcode(void); // New function for barcode payment
void enviar_mensagem_pagamento(const char* banco, const char* convenio, const char* valor);
char receber_resposta_pagamento(void);

// Barcode related functions
char parse_barcode_data(const char* barcode_data, char* banco, char* convenio, char* valor);
int calculate_module10_dv(const char* data, int length);
int calculate_module11_dv(const char* data, int length);

#endif /* OPERACAO_H */