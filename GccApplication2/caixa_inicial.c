/* Biblioteca para exibir abertura de caixa 
Aguarda at� que algo seja pressionado 
� chamada sempre que perder a comunica��o com o usu�rio 
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/
#include "timers.h"
#include "LCD.h"
#include "teclado.h"
void mensagem_Inicial(){

		LCD_texto_correndo(0, " Banco UFRGS ", 300, 0);
		LCD_Escrever_Linha(1, 0, "Digite cartao: ");

	
}