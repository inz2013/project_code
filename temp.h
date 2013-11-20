/*
 * temp.h
 *
 *  Created on: Jul 20, 2013
 *      Author: Piotrus
 */

#ifndef TEMP_H_
#define TEMP_H_
#include "stm32f10x.h"


void uart_config();
void uart_send(uint8_t data);
uint8_t uart_receive();
int ds_init(); //iinicjalizacja scalaka
void uart_flush();
int OWReset(); //reset lini 1-wire
int OWSearch(unsigned char *ROM_ID); //szukanie pojedynczego czujnika
int OWSearch_all(unsigned char ROM_ID[][8]); //wyszukanie wszystkich czujnikow
int bitacc(int op, int state, int loc, unsigned char *buf);
int OWTempConvert_single(unsigned char* ROM_ID); //funkcja konwertuje temp wybranego czujnika
unsigned char docrc8(unsigned char value); //sprawdzanie sumy kontrolnej
int OWTempConvert();//konwersja temperatury wszystkich czujnikow na lini
int ReadTemp(unsigned char *ROM_ID, uint8_t *temp);
void delay_ms(unsigned int delay); //opoznienie w ms - potrzebne bo w sekcji krytycznej wylaczana jest vTaskDelay

#endif /* TEMP_H_ */
