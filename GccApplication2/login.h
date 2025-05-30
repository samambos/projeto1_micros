#ifndef LOGIN_H_
#define LOGIN_H_

#include <stdint.h>  // Adicionado para uint8_t

void ler_codigo_aluno(char* codigo);
void ler_senha(char* senha);
int validar_codigo_aluno(const char* codigo, const char* senha);
void clear_stored_password();
const char* get_current_password();

#endif /* LOGIN_H_ */