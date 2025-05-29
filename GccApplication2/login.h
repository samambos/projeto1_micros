/*
 * login.h
 *
 * Created: 29/05/2025 13:24:34
 *  Author: Usuario
 */ 


#ifndef LOGIN_H_
#define LOGIN_H_

void ler_codigo_aluno(char* codigo);
void ler_senha(char* senha);
int validar_codigo_aluno(const char* codigo, const char* senha);



#endif /* LOGIN_H_ */