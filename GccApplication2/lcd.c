/* Biblioteca para controle de LCD
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "timers.h"

#define portaLCD PORTC	//Entrada dos pinos de controle (A0 a A3)
#define RS PC5			//RS
#define EN PC4 			//E

// Fun��o para comandos do LCD 
void LCD_cmd(unsigned char cmd)
{
	portaLCD = (portaLCD & 0xF0) | (cmd >> 4); // Envia os 4 bits mais significativos 
	portaLCD &= ~ (1<<RS); // Define RS como comando 
	portaLCD |= (1<<EN); // Ativa EN
	portaLCD &= ~ (1<<EN); // Desativa EN
	delay1us(100); // Atraso para o envio

	portaLCD = (portaLCD & 0xF0) | (cmd & 0x0F); // Envia os 4 bits menos significativos 
	portaLCD |= (1<<EN); // Ativa EN 
	portaLCD &= ~ (1<<EN); // Desativa EN
	delay1ms(2); // Atraso para envio
}

// Fun��o chamada na main para inicializa��o do LCD
void LCD_iniciar(void)
{
	DDRC = 0xFF; // Configura como sa�da todos os pinos da PC
	delay1ms(10);
	LCD_cmd(0x02); // Retorna cursor
	LCD_cmd(0x28); // Modo 4 bits, 2 linhas
	LCD_cmd(0x0C); // Display ligado, cursor off
	LCD_cmd(0x06); // Incremento autom�tico
	LCD_cmd(0x01); // Limpa display
}

// Fun��o para limpar display
void LCD_limpar(void)
{
	LCD_cmd(0x01); // Limpa display
	LCD_cmd(0x80); // Posiciona no in�cio
}

// Fun��o para escrever, sem posicionamento das linhas 
void LCD_escrever(const char *texto)
{
	while(*texto) {
		portaLCD = (portaLCD & 0xF0) | (*texto >> 4); // 4 bits mais significativos 
		portaLCD |= (1<<RS); // Modo dado
		portaLCD |= (1<<EN); // Ativa EN 
		portaLCD &= ~(1<<EN); // Desativa EN 
		delay1us(200);

		portaLCD = (portaLCD & 0xF0) | (*texto & 0x0F); // 4 bits menos significativos 
		portaLCD |= (1<<EN); // Ativa EN 
		portaLCD &= ~(1<<EN); // Desativa EN 
		delay1ms(2);
		texto++;
	}
}

// Fun��o para escrever, com posicionamento das linhas 
void LCD_Escrever_Linha (char linha, char pos,const char *texto)
{
	// Verifica se a linha � 0 e a posi��o � v�lida 
	if (linha == 0 && pos < 16){
		LCD_cmd((pos & 0x0F) | 0x80);  // Envia o comando para posicionar o cursor na primeira linha
	}

	// Verifica se a linha � 1 e a posi��o � v�lida 
	else if (linha == 1 && pos < 16){
		LCD_cmd((pos & 0x0F) | 0xC0);  // Envia o comando para posicionar o cursor na segunda linha
	}

	LCD_escrever(texto);	// Chama a fun��o para imprimir a string a partir da posi��o especificada
	LCD_cmd(0x80);		// Retorna cursor para posi��o inicial
}

// Fun��o para fazer texto correr no display LCD
void LCD_texto_correndo(uint8_t linha, const char *texto, uint16_t velocidade_ms, uint8_t loop) {
    uint8_t tamanho = strlen(texto);
    char buffer[17]; // Buffer para o display
    
    // Se o texto for menor que 16 caracteres, n�o precisa fazer scroll
    if(tamanho <= 16) {
        LCD_Escrever_Linha(linha, 0, texto);
        return;
    }
    
    // Adiciona espa�os no final para melhor visualiza��o
    char texto_extendido[tamanho + 17];
    strcpy(texto_extendido, texto);
    strcat(texto_extendido, "                "); // 16 espa�os
    
    do {
        for(uint8_t i = 0; i <= tamanho; i++) {
            // Copia 16 caracteres come�ando na posi��o i
            strncpy(buffer, &texto_extendido[i], 16);
            buffer[16] = '\0';
            
            LCD_Escrever_Linha(linha, 0, buffer);
            delay1ms(velocidade_ms);
            
            // Verifica se alguma tecla foi pressionada para sair
            if(varredura() != 0 && !loop) return;
        }
    } while(loop);
}