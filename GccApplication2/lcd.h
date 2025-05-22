/* Arquivo .h para controle de LCD
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/

#ifndef LCD_H_
#define LCD_H_

void LCD_limpa();
void LCD_cmd( unsigned char cmnd );
void LCD_iniciar(void);
void LCD_escrever(const char *printando);
void LCD_Escrever_Linha (char linha, char pos,const char *printando);
void LCD_texto_correndo(char linha, const char *texto, char velocidade_ms, char loop);

#endif /* LCD_H_ */