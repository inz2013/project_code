/*
 * lcd.c
 *
 *  Created on: Apr 10, 2012
 *      Author: Piotr Hajdukiewicz
 */

#include <stddef.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "FRTOS/FreeRTOS.h"
#include "FRTOS/task.h"
#include "FRTOS/queue.h"
#include "FRTOS/portmacro.h"
#include "FRTOS/list.h"
#include "data_model.h"

const char *miesiace[12] = { "sty", "lut", "mar", "kwi", "maj", "czer", "lip", "sie", "wrz", "paz", "lis", "gru" };

//======INICJALIZACJA WYSWIETLACZA=====================
void eval_init() // funkcja inicjalizujaca porty plytki odpowiedzialne za obsluge LCD
{
	RCC_APB2PeriphClockCmd(LCD_PORT_CLK, ENABLE);// wlaczenie zegara portu wysiwetlacza
	RCC_APB2PeriphClockCmd(JOY_PORT_CLK, ENABLE);//wlaczenie zegara portu joystika


	GPIO_InitTypeDef  GPIO_InitStructure; //struktura inicjalizacyjna portu

	GPIO_InitStructure.GPIO_Pin = LCD_D7 | LCD_D6 | LCD_D5 | LCD_D4 | LCD_RS | LCD_RW | LCD_EN | LCD_DIR; //piny które chcemy skonfigurowaæ
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//tryb wyjœæ portu - wyjœcie push-pull
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 	//szybkoœæ wyjœcia
	GPIO_Init(LCD_PORT, &GPIO_InitStructure); //inicjalizacja portu

	GPIO_InitStructure.GPIO_Pin = JOY_OK; //piny które chcemy skonfigurowaæ
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//tryb wyjœæ portu - wyjœcie push-pull
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 	//szybkoœæ wyjœcia
	GPIO_Init(LCD_PORT, &GPIO_InitStructure);

	GPIO_SetBits(LCD_PORT, LCD_DIR);
}

//=======WYSLANIE BAJTU DO WYSWIETLACZA=====
void LCD_SendByte(uint8_t cmd)
{
	lastTime = xTaskGetTickCount();
	GPIO_SetBits(LCD_PORT, LCD_DIR); //ustawienie do "regenerowania" napiecia w strone wyswietlacza

	GPIO_ResetBits(LCD_PORT, LCD_RW); //ustawienie wyswietlacza na odbior RW=0
	GPIO_ResetBits(LCD_PORT, LCD_D7 | LCD_D6 | LCD_D5 | LCD_D4); //wyzerowanie pinow danych
	GPIO_ResetBits(LCD_PORT, LCD_EN); //ENABLE = 0

	//zapis starszej polowy
	if( cmd & 0x10 )
		GPIO_SetBits(LCD_PORT, LCD_D4);
	if( cmd & 0x20 )
		GPIO_SetBits(LCD_PORT, LCD_D5);
	if( cmd & 0x40 )
		GPIO_SetBits(LCD_PORT, LCD_D6);
	if( cmd & 0x80 )
		GPIO_SetBits(LCD_PORT, LCD_D7);

	GPIO_SetBits(LCD_PORT, LCD_EN);//enalbe = 1
	vTaskDelayUntil(&lastTime, 1/portTICK_RATE_MS);
	GPIO_ResetBits(LCD_PORT, LCD_EN); //ENABLE=0

	vTaskDelayUntil(&lastTime, 1/portTICK_RATE_MS); // opoznienie


	GPIO_ResetBits(LCD_PORT, LCD_D7 | LCD_D6 | LCD_D5 | LCD_D4); //wyzerowanie pinow danych

	//zapis mlodszej polowy
	cmd &= 0x0F; //maskowanie czterech mlodszych bitow
	if( cmd & 0x01 )
		GPIO_SetBits(LCD_PORT, LCD_D4);
	if( cmd & 0x02 )
		GPIO_SetBits(LCD_PORT, LCD_D5);
	if( cmd & 0x04 )
		GPIO_SetBits(LCD_PORT, LCD_D6);
	if( cmd & 0x08 )
		GPIO_SetBits(LCD_PORT, LCD_D7);

	GPIO_SetBits(LCD_PORT, LCD_EN); //ENABLE = 1
	vTaskDelayUntil(&lastTime, 1/portTICK_RATE_MS);
	GPIO_ResetBits(LCD_PORT, LCD_EN); //ENABLE=0
	GPIO_ResetBits(LCD_PORT, LCD_D7 | LCD_D6 | LCD_D5 | LCD_D4); //wyzerowanie pinow danych
	GPIO_ResetBits(GPIOB, LCD_RS);

	vTaskDelayUntil(&lastTime, 2/portTICK_RATE_MS); //oczekiwanie na zakonczenie operacji
}

//===================WYSLANIE BAJTU POLECENIA DO WYSWIETLACZA==============
void LCD_SendCmd(uint8_t cmd)
{
	GPIO_ResetBits(LCD_PORT, LCD_RS); //ustawienie wysiwetlacza w tryb odbioru polecen

	LCD_SendByte(cmd); //wyslanie bajtu polecenia
}

//=================WYSLANIE BAJTU DANYCH DO WYSWIETLACZA=====================
void LCD_SendData(uint8_t data)
{
	GPIO_SetBits(LCD_PORT, LCD_RS);// RS=1
	GPIO_ResetBits(LCD_PORT, LCD_RW); //RW=0

	LCD_SendByte(data);
}

//====================USTAWIENIE POZYCJI KURSORA NA WYSWIETLACZU===================
void LCD_GoTo(uint8_t wers, uint8_t kol) //wers <1,4> kol<0,15>
{
	uint8_t adres; //ustalany adres kursora

	if(wers == 1){
		adres = kol;
	}
	else if(wers == 2){
		adres = 64 + kol;
	}
	else if(wers == 3){
		adres = 16 + kol;
	}
	else if(wers == 4){
		adres = 80 + kol;
	}
	else adres = 0;

	adres = adres + 128; //ustalenie dotowej zmiennej do wyslania do wysiwtlacza, trzeba przed adresem bac 1 w zapisie binarnym

	LCD_SendCmd(adres); //ustalenie adresu
}

//========================WYSLANIE LANCUCHA ZNAKOW DO WYSWIETLACZA=================
void LCD_SendText(const char *text)
{
/*	GPIO_SetBits(LCD_PORT, LCD_RS);// RS=1
	GPIO_ResetBits(LCD_PORT, LCD_RW); //RW=0

	int i = 0;

	while(text[i]!='\0'){
		LCD_SendByte(text[i]);
		i++;
	}*/

	while(*text)
	{
		LCD_SendData(*text);
		text++;
	}
}

////=================OPOZNIENIE W ms=========================================
//void delay_ms(unsigned int delay)
//{
//  volatile int i; //zmienna typu volatile
//  for(;delay>0;delay--)
//    for(i=5000;i>0;i--);
//}

//=====================WYCZYSZCZENIE WYSWIETLACZA==============================
void LCD_clear()
{
	LCD_SendCmd(0x1); //wyslanie "000000001" - wyczyszczenie wyswietlacza
}

//======================INICJALIZACJA WYSWIETLACZA=================================
void LCD_Init()
{
//	lastTime = xTaskGetTickCount();
//
//	vTaskDelayUntil(&lastTime, 100/portTICK_RATE_MS); //oczekiwanie na zasilanie
//	GPIO_SetBits(LCD_PORT, LCD_DIR);
//	//LCD_SendCmd(0x30); // wyslanie "00110000" Function Set
//	//wyslanie pierwieszego rozkazu
//
//	GPIO_ResetBits(LCD_PORT, LCD_RS);
//	GPIO_ResetBits(LCD_PORT, LCD_RW); //ustawienie wyswietlacza na odbior RW=0
//	GPIO_ResetBits(LCD_PORT, LCD_DB7 | LCD_DB6 | LCD_DB5 | LCD_DB4); //wyzerowanie pinow danych
//	GPIO_ResetBits(LCD_PORT, LCD_EN); //ENABLE = 0
//
//
//
//	GPIO_ResetBits(LCD_PORT, LCD_RS);
//	GPIO_ResetBits(LCD_PORT, LCD_RW);
//	GPIO_ResetBits(LCD_PORT, LCD_DB7);
//	GPIO_ResetBits(LCD_PORT, LCD_DB6);
//	GPIO_SetBits(LCD_PORT, LCD_DB5);
//	GPIO_SetBits(LCD_PORT, LCD_DB4);
//
//	GPIO_SetBits(LCD_PORT, LCD_EN);//enable = 1
//	vTaskDelay(1/portTICK_RATE_MS);
//	GPIO_ResetBits(LCD_PORT, LCD_EN); //ENABLE = 0
//	GPIO_ResetBits(LCD_PORT, LCD_DB7 | LCD_DB6 | LCD_DB5 | LCD_DB4);
//
//
//
//	vTaskDelayUntil(&lastTime, 1/portTICK_RATE_MS);
//	LCD_SendCmd(0x2C); //wyslanie "00101100" Function Set
//	vTaskDelayUntil(&lastTime, 1/portTICK_RATE_MS);
//	LCD_SendCmd(0x2C);// wyslanie "00101100" Function Set
//	vTaskDelayUntil(&lastTime, 1/portTICK_RATE_MS);
//	LCD_SendCmd(0xF); // wyslanie "00001111" Display Control, migajacy kursor
//	vTaskDelayUntil(&lastTime, 1/portTICK_RATE_MS);
//	LCD_clear();
//	vTaskDelayUntil(&lastTime, 3/portTICK_RATE_MS);
//	LCD_SendCmd(0x6); //wyslanie "00000110" Entry mode set

	uint8_t i = 0;
	vTaskDelay(100/portTICK_RATE_MS);

  	GPIO_SetBits(GPIOB, LCD_DIR);

	GPIO_ResetBits(GPIOB, LCD_RS);
	GPIO_ResetBits(GPIOB, LCD_EN);

	for(i = 0; i<3; i++)
	{
		GPIO_SetBits(GPIOB, LCD_EN);
		GPIO_SetBits(GPIOB, LCD_D4 | LCD_D5);
		GPIO_ResetBits(GPIOB, LCD_D6 | LCD_D7);
		GPIO_ResetBits(GPIOB, LCD_EN);
		vTaskDelay(5/portTICK_RATE_MS);
	}

	GPIO_SetBits(GPIOB, LCD_EN);
	GPIO_SetBits(GPIOB, LCD_D5);
	GPIO_ResetBits(GPIOB,LCD_D4 | LCD_D6 | LCD_D7);
	GPIO_ResetBits(GPIOB, LCD_EN);
	vTaskDelay(1/portTICK_RATE_MS);
	LCD_SendCmd(0x2C);
	vTaskDelay(1/portTICK_RATE_MS);
	LCD_SendCmd(0x2C);
	vTaskDelay(1/portTICK_RATE_MS);
	LCD_SendCmd(0xF);
	vTaskDelay(1/portTICK_RATE_MS);
	LCD_SendCmd(0x01);
	vTaskDelay(1/portTICK_RATE_MS);
	LCD_SendCmd(0x06);
	vTaskDelay(1/portTICK_RATE_MS);
}

void display_time(RTC_t time, uint8_t x, uint8_t y, const char *buf)
{

	LCD_GoTo(x,y);

	//poczatek godziny
	if(time.hour<10){
	LCD_SendText("0");
	LCD_SendText(itoa(time.hour, buf));
	}
	else  LCD_SendText(itoa(time.hour, buf));

	LCD_SendText(":");

	if(time.min<10){
	LCD_SendText("0");
	LCD_SendText(itoa(time.min, buf));
	}
	else LCD_SendText(itoa(time.min, buf));

	LCD_SendText(":");

	if(time.sec<10){
	LCD_SendText("0");
	LCD_SendText(itoa(time.sec, buf));
	}
	else LCD_SendText(itoa(time.sec, buf));
	//koniec godziny
}

void display_date(RTC_t time, uint8_t x, uint8_t y, const char *buf)
{
	LCD_clear();
	LCD_GoTo(x,y);

	//poczatek daty

	if(time.mday<10){
	LCD_SendText("0");
	LCD_SendText(itoa(time.mday, buf));
	}
	else  LCD_SendText(itoa(time.mday, buf));

	LCD_SendText(" ");

//	if(time.month<10){
//	LCD_SendText("0");
//	LCD_SendText(itoa(time.month, buf));
//	}
//	else LCD_SendText(itoa(time.month, buf));
	LCD_SendText(miesiace[time.month-1]);

	LCD_SendText(" ");

	if(time.year<10){
	LCD_SendText("0");
	LCD_SendText(itoa(time.year, buf));
	}
	else LCD_SendText(itoa(time.year, buf));

	//koniec daty
}

char* itoa(int n, char s[])
{
   char i,c,j;
                int sign;
   if ((sign = n) < 0) n = -n;
   i = 0;
   do {
      s[i++] = n % 10 + '0';
   } while ((n /= 10) > 0);
   if (sign < 0)
      s[i++] = '-';
   s[i] = 0;
   for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
      c = s[i];
      s[i] = s[j];
      s[j] = c;
   }
   return s;
}










