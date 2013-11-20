/*
 * lcd.h
 *
 *  Created on: Apr 6, 2012
 *      Author: Piotr Hajdukiewicz
 */

#include <stddef.h>
#include "stm32f10x.h"
#include "FRTOS/FreeRTOS.h"
#include "FRTOS/task.h"
#include "FRTOS/queue.h"
#include "FRTOS/portmacro.h"
#include "FRTOS/list.h"
#include "rtc.h"

#ifndef LCD_H_
#define LCD_H_



#define LCD_PORT			GPIOB	 //port do ktorego bedzie podlaczony wyswietlacz
#define LCD_PORT_CLK		RCC_APB2Periph_GPIOB //zegar portu
#define JOY_PORT			GPIOC //port joystika
#define JOY_PORT_CLK		RCC_APB2Periph_GPIOC // zegar portu joystika

//PINY STERUJACE PRACA WYSWIETLACZA
#define LCD_RS				GPIO_Pin_10 // pin RS wyswietlacza
#define LCD_RW				GPIO_Pin_11 // pin R/W wyswietlacza
#define LCD_EN				GPIO_Pin_9 // pin ENABLE wyswietlacza
#define LCD_DIR					GPIO_Pin_8 // sterowanie buforem


// szyna danych wyswietlacza
#define LCD_D7				GPIO_Pin_15
#define LCD_D6				GPIO_Pin_14
#define LCD_D5				GPIO_Pin_13
#define LCD_D4				GPIO_Pin_12

//JOYSTIK

#define JOY_OK				GPIO_Pin_5
#define JOY_UP				GPIO_Pin_0
#define JOY_DOWN			GPIO_Pin_1
#define JOY_L				GPIO_Pin_2
#define JOY_R				GPIO_Pin_3



//funkcje

void delay_ms(unsigned int delay); // funkcja opozniajaca argument: czas w milisekundach o ktory ma procesor byc opozniony


void eval_init();//inicjalizacja portow plytki na wyjsciowe
void LCD_Init(); //inicjalizacja wyswietlacza

void LCD_clear(); //czysci wyswietlacz

void LCD_SendByte(uint8_t cmd);//wyslanie bajtu do wyswietlacza
void LCD_SendCmd(uint8_t cmd);//wyslanie polecenia do wyswietlacza
void LCD_SendData(uint8_t data); //wyslanie bajtu danych do wyswietlacza
void LCD_SendText(const char *text);//wyslanie lancucha znakow
void LCD_GoTo(uint8_t wers, uint8_t kol); // przenosi kursor do pozycji (wers,kol)
void display_time(RTC_t time, uint8_t x, uint8_t y, const char* buf);
void display_date(RTC_t time, uint8_t x, uint8_t y, const char* buf);
char* itoa(int n, char s[]);
portTickType lastTime; ///do odmierzania czasu

typedef struct
{
	long xColumn;
	signed char *pcMessage;
} xLCDMessage;


#endif /* LCD_H_ */
