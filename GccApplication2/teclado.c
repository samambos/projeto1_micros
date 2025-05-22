/* Biblioteca para controle de teclado matricial
Projeto 1: Alice, Carlos e Samanta
Disciplina: Microcontroladores 
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "timers.h"
#include "teclado.h"

// Define os pinos do teclado matricial (verificar pinagem do arduino para ligar hardware)
#define LIN1 PB1    // Linha 1 no PB1 
#define LIN2 PB0    // Linha 2 no PB0
#define LIN3 PD7    // Linha 3 no PD7
#define LIN4 PD6    // Linha 4 no PD6
#define COL1 PD5    // Coluna 1 no PD5
#define COL2 PD4    // Coluna 2 no PD4
#define COL3 PD3    // Coluna 3 no PD3

// Função inicial a ser chamada na main para inicialização 
void prepara_teclado(){
    // Configura linhas como saída
    DDRB |= (1 << LIN1) | (1 << LIN2);      // PB1 e PB0 como saídas
    DDRD |= (1 << LIN3) | (1 << LIN4);      // PD7 e PD6 como saídas
    
    // Configura colunas como entrada com pull-up
    DDRD &= ~((1 << COL1) | (1 << COL2) | (1 << COL3)); // PD5, PD4 e PD3 como entradas
    PORTD |= (1 << COL1) | (1 << COL2) | (1 << COL3);   // Habilitar resistores de Pull-up
}

// Função para tempo de bounce da tecla 
void atraso_debounce(){
    volatile unsigned short s;
    for(s=0;s<1000;s++); 
}

// Função para debounce das teclas 
char debounce (char pino) {
    char teclaAtual = 0, ultimaTecla = 0, counter = 0;
    char const BOUNCE = 7;
    while(counter != BOUNCE){
        atraso_debounce();
        teclaAtual = PIND & (1 << pino); // Lê o pino específico em PORTD
        if(ultimaTecla == teclaAtual){
            counter++;
        } else {
            counter = 0;
        }
        ultimaTecla = teclaAtual;
    }
    return teclaAtual;
}

// Função para varredura do teclado matricial 
char varredura() {
    char tecla = 0;
    
    // Desativa todas as linhas
    PORTB |= (1 << LIN1) | (1 << LIN2);
    PORTD |= (1 << LIN3) | (1 << LIN4);

    // Linha 1 (PB1)
    PORTB &= ~(1 << LIN1); // Ativa LIN1 (nível baixo)
    if (!debounce(COL1 - 0)) tecla = '1'; // COL1 = PD5 (5-0=5)
    if (!debounce(COL2 - 0)) tecla = '2'; // COL2 = PD4 (4-0=4)
    if (!debounce(COL3 - 0)) tecla = '3'; // COL3 = PD3 (3-0=3)
    PORTB |= (1 << LIN1); // Desativa LIN1

    // Linha 2 (PB0)
    PORTB &= ~(1 << LIN2); // Ativa LIN2 (nível baixo)
    if (!debounce(COL1 - 0)) tecla = '4'; // COL1 = PD5
    if (!debounce(COL2 - 0)) tecla = '5'; // COL2 = PD4
    if (!debounce(COL3 - 0)) tecla = '6'; // COL3 = PD3
    PORTB |= (1 << LIN2); // Desativa LIN2

    // Linha 3 (PD7)
    PORTD &= ~(1 << LIN3); // Ativa LIN3 (nível baixo)
    if (!debounce(COL1 - 0)) tecla = '7'; // COL1 = PD5
    if (!debounce(COL2 - 0)) tecla = '8'; // COL2 = PD4
    if (!debounce(COL3 - 0)) tecla = '9'; // COL3 = PD3
    PORTD |= (1 << LIN3); // Desativa LIN3

    // Linha 4 (PD6)
    PORTD &= ~(1 << LIN4); // Ativa LIN4 (nível baixo)
    if (!debounce(COL1 - 0)) tecla = '*'; // COL1 = PD5
    if (!debounce(COL2 - 0)) tecla = '0'; // COL2 = PD4
    if (!debounce(COL3 - 0)) tecla = '#'; // COL3 = PD3
    PORTD |= (1 << LIN4); // Desativa LIN4

    return tecla; // Retorna a tecla identificada ou 0 se nenhuma foi pressionada
}