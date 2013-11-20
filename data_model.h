/*
 * data_model.h
 *
 *  Created on: Aug 18, 2013
 *      Author: Piotrus
 */
#include "FRTOS/FreeRTOS.h"
#include "FRTOS/task.h"
#include "FRTOS/queue.h"
#include "rtc.h"
#include "FRTOS/semphr.h"

#ifndef DATA_MODEL_H_
#define DATA_MODEL_H_
#define AMOUNT 2   //liczba czujnikow na lini 1-Wire
#define MEAS_ON 1 //pomiary w trakcie
#define MEAS_OFF 0 //brak pomiarow w trakcje
#define SD_ON 1 //karta sd aktywna
#define SD_OFF 0 //karta sd nieaktywna


uint8_t status_pomiarow; //czy sa piomiary czy nie
uint8_t status_SD; //czy karta sd jest aktywna
uint8_t ROM_ID[AMOUNT][8]; //tablica czujnikow

uint8_t END_FLAG; //flaga zakonczenia zadania pomiarowego

xSemaphoreHandle xSemaphoreMeas; //semafor do blokowania "sekcji krytycznej" pomiaru
xSemaphoreHandle xSemaphoreMeasQueue; //semafor do blokowania dostepu do kolejki pomiarowej

//handlery watkow
xTaskHandle xHandleTaskLCD_JOY,
			xHandleTaskSD,
			xHandleTaskMeas,
			xHandleSensor[AMOUNT];

//handlery kolejek
xQueueHandle xLogQueue,
			 xMeasQueue,
			 xTaskQueue;

uint8_t ktore_zadanie; //numer zadania pomiarowego dla danego dnia

//struktura z informacja do pilku log.txt
typedef struct
{
	char *logMessage; //informacja do zapisu
	RTC_t czas; //czas informacji do loga
} xLogMessage;

//struktura z danymi pomiarowymi czujnika
typedef struct
{
	uint8_t numer_zadania; //numer aktualnie wykonywanego zadania pomiarowego
	RTC_t czas_pomiaru; //czas w ktorym zostal wykonany pomiar
	uint8_t numer_czujnika; //pozycja id czujnika w tablicy z id
	uint8_t pomiar; //wartosc pomiaru

} xMeasMessage;

//struktura z konfiguracja pomiarow
typedef struct
{
	uint8_t numer_czujnika; //numer czujnika - pozycja id czujnika w tablicy z id
	uint8_t liczba_pomiarow; //liczba pomiarow do wykonania przez dany czujnik
	uint8_t interwal; //odstep czasu pomiedzy pomiarami
	uint8_t numer_zadania;

} meas_conf;

typedef struct
{
	uint8_t numer_zadania;
	meas_conf konfiguracje_czujnikow[AMOUNT]; //zbior wszystkich konfiguracji czujnikow
} zadanie_pomiarowe;

//struktura do budowy menu
typedef struct
{
	uint8_t pozycja; 					// pozycja kursora
	uint8_t next_state[6];				//przejœcia do nastêpnych stanów
	void (*callback)(uint8_t event);	//funkcja zwrotna
	const char* display[4];				//tekst dla 4 linii LCD

} menu_item;

//struktura do budowy menu
typedef struct
{
	zadanie_pomiarowe zadanie; //zadanie pomiarowe czekajace na wykonanie
	uint8_t 		  zakonczone; //1 - tak, 0 - nie

} element_kolejki;

element_kolejki kolejka_zadan[3]; //lista zadan do wykonania

#endif /* DATA_MODEL_H_ */
