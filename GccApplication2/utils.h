// utils.h
#ifndef UTILS_H
#define UTILS_H

// Declaração dos estados para manter compatibilidade com utils.c
typedef enum {
	STANDBY,
	AUTENTICACAO,
	MENU,
	OPERACAO,
	ERROR_STATE
} Estado;

extern Estado estado_atual;

// Funções utilitárias
void aguardar_desbloqueio(void);
int ler_codigo_aluno(char* codigo);
int ler_senha(char* senha);
int validar_codigo_aluno(const char* codigo, const char* senha);
void atualiza_estado(Estado proximo);

#endif
