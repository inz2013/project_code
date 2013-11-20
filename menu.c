#include <stddef.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "lcd.h"
#include "FRTOS/portmacro.h"
#include "FRTOS/list.h"
#include "FRTOS/semphr.h"
#include "rtc.h"
#include "menu.h"
#include "SD_card.h"
#include "FRTOS/task.h"


volatile uint8_t	current_menu = 0;
volatile uint8_t	menu_event = E_IDLE;
RTC_t czas_odebrany, czas;
char buf[5]; // do konwersji liczba na string do wyswietlania
uint8_t flaga_kolejki = 0;


uint16_t rok_u;// do ustawianie roku
uint8_t data_godzina_u[4]; //dzien, miesiac, godziny, minuty
uint8_t meas_period[AMOUNT];
uint8_t meas_number[AMOUNT];
uint8_t aktualny_czujnik = 0;

void vTaskLCD_JOY(void* pvParameters){

	ktore_zadanie = 1;
	uint8_t k =0;
	for (k=0;k<AMOUNT;k++)
	{
		meas_period[k] = 1;
		meas_number[k] = 1;
	}

	vSemaphoreCreateBinary(xSemaphoreDOWN);
	xSemaphoreTake(xSemaphoreDOWN, 0);
	vSemaphoreCreateBinary(xSemaphoreUP);
	xSemaphoreTake(xSemaphoreUP, 0);
	vSemaphoreCreateBinary(xSemaphoreOK);
	xSemaphoreTake(xSemaphoreOK, 0);
	vSemaphoreCreateBinary(xSemaphoreLEFT);
	xSemaphoreTake(xSemaphoreLEFT, 0);
	vSemaphoreCreateBinary(xSemaphoreRIGHT);
	xSemaphoreTake(xSemaphoreRIGHT, 0);

	const menu_item menu[] = {
		    // U, D, O, I, R, L
		{ 2, { 0, 1, 16, 0, 0, 0}, NULL, {" ", "Nowe zadanie", "Lista zadan", "Konfiguracja"} }, // pomiary 0
		{ 3, { 0, 2, 20, 1, 1, 1}, NULL, {" ", "Nowe zadanie", "Lista zadan", "Konfiguracja"} }, // lista zadan 1
		{ 4, { 1, 2, 3, 2, 2, 2}, NULL, {" ", "Nowe zadanie", "Lista zadan", "Konfiguracja"} }, // konfiguracja 2
		{ 2, { 3, 4, 6, 3, 3, 3}, NULL, {" ", "Data/godzina", "Czujniki", "Cofnij"} },  //data/godzina 3
		{ 3, { 3, 5, 4, 4, 4, 4}, NULL, {" ", "Data/godzina", "Czujniki", "Cofnij"} },  //czujniki 4
		{ 4, { 4, 5, 0, 5, 5, 5}, NULL, {" ", "Data/godzina", "Czujniki", "Cofnij"} },  //Cofnij(konfiguracja) 5
		{ 3, { 6, 7, 8, 6, 6, 6}, display_date10, { } },  //data/godzina ogladanie 6
		{ 4, { 6, 7, 3, 7, 7, 7}, display_date10, { } },  //cofanie z data/godzina ustawianie 7
		{ 1, { 8, 9, 8, 8, 8, 8}, set_day, {"Dzien:          ", "Miesiac:", "Rok:", "Godzina:"} },  //data/godzina ustawianie 8
		{ 2, { 8, 10, 9, 9, 9, 9}, set_month, {"Dzien:", "Miesiac:", "Rok:", "Godzina:"} },  //data/godzina ustawianie 9
		{ 3, { 9, 11, 10, 10, 10, 10}, set_year, {"Dzien:", "Miesiac:", "Rok:", "Godzina:"} }, //data/godzina ustawianie 10
		{ 4, { 10, 12, 11, 11, 11, 11}, set_hour, {"Dzien:", "Miesiac:", "Rok:", "Godzina:"} },  //data/godzina ustawianie 11
		{ 4, { 11, 13, 12, 12, 12, 12}, set_min, {"Miesiac:", "Rok:", "Godzina:", "Minuty:"} },  //data/godzina ustawianie 12
		{ 4, { 12, 14, 15, 13, 13, 13}, NULL, {"Rok:", "Godzina:", "Minuty", "=Ustaw="} },  //data/godzina ustawianie 13
		{ 4, { 13, 14, 6, 14, 14, 14}, NULL, {"Godzina:", "Minuty:", "=Ustaw=", "=Cofnij="} },   //data/godzina ustawianie 14
		{ 4, { 15, 15, 15, 15, 15, 15}, change_date, {"Godzina:", "Minuty:", "=Ustaw=", "=Cofnij="} },  //zatwierdzanie ustawien 15
		{ 2, { 16, 17, 16, 16, 16, 16}, set_period, {"**Czujnik***", "Interwal: ", "Ilosc: ", "=Dalej=" } },  //ustawienie zadania pomiarowego (interwal) 16
		{ 3, { 16, 18, 17, 17, 17, 17}, set_meas_number, {"**Czujnik***", "Interwal: ", "Ilosc: ", "=Dalej=" } },  //ustawienie nowego zadania (ilosc pomiarow) 17
		{ 4, { 17, 18, 19, 18, 18, 0}, NULL, {"**Czujnik***", "Interwal: ", "Ilosc: ", "=Dalej=" } },  //ustawienie nowego zadania (dalej) 18
		{ 4, { 19, 19, 19, 19, 19, 0}, next_or_accept, {"**Czujnik***", "Interwal: ", "Ilosc: ", "=Dalej=" } },  //ustawienie nowego zadania (dalej - wywolanie funkcji) 19
		{ 4, { 20, 20, 20, 20, 20, 0}, check_queue, { } } //20, zadan pomiarowych
	};

	LCD_Init();

	LCD_GoTo(2,1);
	LCD_SendText("Nowe zadanie");
	LCD_GoTo(3,1);
	LCD_SendText("Lista zadan");
	LCD_GoTo(4,1);
	LCD_SendText("Konfiguracja");

	//sd_log("Jestesmy w funkcji LCD");

	while (1)
	{
		//obsluga przyciskow
		if (xSemaphoreTake(xSemaphoreDOWN, 0) == pdTRUE)
		{
			vTaskDelay(150/portTICK_RATE_MS); //debouncing
			menu_event = E_DOWN;
			LCD_clear();
		}
		else if (xSemaphoreTake(xSemaphoreUP, 0) == pdTRUE)
		{
			vTaskDelay(150/portTICK_RATE_MS); //debouncing
			menu_event = E_UP;
			LCD_clear();
		}
		else if (xSemaphoreTake(xSemaphoreOK, 0) == pdTRUE)
		{
			vTaskDelay(150/portTICK_RATE_MS); //debouncing
			menu_event = E_OK;
			LCD_clear();
		}
		else if (xSemaphoreTake(xSemaphoreLEFT, 0) == pdTRUE)
		{
			vTaskDelay(150/portTICK_RATE_MS); //debouncing
			menu_event = E_LEFT;
			LCD_clear();
		}
		else if (xSemaphoreTake(xSemaphoreRIGHT, 0) == pdTRUE)
		{
			vTaskDelay(150/portTICK_RATE_MS); //debouncing
			menu_event = E_RIGHT;
			LCD_clear();
		}
		//koniec obslugi przyciskow

		rtc_gettime(&czas_odebrany);

		if(current_menu < 8)
		{
			LCD_GoTo(1, 3); //wyswietlanie statusu karty
			if (status_SD == SD_ON)
			{
				LCD_SendText("SD");
			}
			else
			{
				LCD_SendText("  ");
			}

//			LCD_GoTo(1, 4); //wyswietlanie statusu karty
//			LCD_SendText(itoa(END_FLAG, buf));

			LCD_GoTo(1, 1); //wyswietlanie statusu pomiarow
			if (status_pomiarow == MEAS_ON)
			{
				GPIO_ResetBits(GPIOB, GPIO_Pin_1);
				LCD_SendText("P");
			}
			else
			{
				GPIO_SetBits(GPIOB, GPIO_Pin_1);
				LCD_SendText(" ");
			}

			//wyswietlanie godziny
			display_time(czas_odebrany, 1, 8, buf);
		}
			//koniec wyswietlania godziny

		if (menu_event !=  E_IDLE )
		{
			change_menu(menu);
		}

		LCD_GoTo(menu[current_menu].pozycja, 0);

		vTaskDelay(300/portTICK_RATE_MS);
	}
}

void change_menu(const menu_item menu[])
{
	//przejdz do nastepnego stanu
	current_menu = menu[current_menu].next_state[menu_event];
	//wywolaj funkcje zwrotna
	if (menu[current_menu].callback){

		LCD_GoTo(1,1);
		LCD_SendText(menu[current_menu].display[0]);
		LCD_GoTo(2,1);
		LCD_SendText(menu[current_menu].display[1]);
		LCD_GoTo(3,1);
		LCD_SendText(menu[current_menu].display[2]);
		LCD_GoTo(4,1);
		LCD_SendText(menu[current_menu].display[3]);

		menu[current_menu].callback(menu_event);
	}

	else {
	//wyswietl komunikat
	LCD_GoTo(1,1);
	LCD_SendText(menu[current_menu].display[0]);
	LCD_GoTo(2,1);
	LCD_SendText(menu[current_menu].display[1]);
	LCD_GoTo(3,1);
	LCD_SendText(menu[current_menu].display[2]);
	LCD_GoTo(4,1);
	LCD_SendText(menu[current_menu].display[3]); }


	//skasuj zdarzenie
	menu_event = E_IDLE;
}

void display_date10(uint8_t event)
{
		rok_u = czas_odebrany.year;
		data_godzina_u[0] = czas_odebrany.mday;
		data_godzina_u[1] = czas_odebrany.month;
		data_godzina_u[2] = czas_odebrany.hour;
		data_godzina_u[3] = czas_odebrany.min;


		LCD_GoTo(2,1);
		display_date(czas_odebrany, 2, 2, buf);
		LCD_GoTo(3,1); LCD_SendText("=Ustaw=");
		LCD_GoTo(4,1); LCD_SendText("=Cofnij=");
}


void set_year(uint8_t event){

	LCD_GoTo(3, 11);

	switch (menu_event)
	{
	case E_LEFT:
		rok_u--;
		break;
	case E_RIGHT:
		rok_u++;
		break;
	}

	LCD_SendText(itoa(rok_u, buf));
}

void set_day(uint8_t event)
{
	LCD_GoTo(1, 11);

	switch (menu_event)
	{
	case E_LEFT:
		data_godzina_u[0]--;
		if(data_godzina_u[0] == 0xFF) data_godzina_u[0] = 31;
		break;
	case E_RIGHT:
		data_godzina_u[0]++;
		data_godzina_u[0] %= 31;
		break;
	}

	LCD_SendText(itoa(data_godzina_u[0], buf));
}


void set_month(uint8_t event)
{
	LCD_GoTo(2, 11);

	switch (menu_event)
	{
	case E_LEFT:
		data_godzina_u[1]--;
		break;
	case E_RIGHT:
		data_godzina_u[1]++;
		break;
	}

	LCD_SendText(itoa(data_godzina_u[1], buf));
}
void set_hour(uint8_t event)
{
	sd_log("wlezlismy do ustawiania godziny");
	LCD_GoTo(4, 11);

	switch (menu_event)
	{
	case E_LEFT:
		data_godzina_u[2]--;
		break;
	case E_RIGHT:
		data_godzina_u[2]++;
		break;
	}

	LCD_SendText(itoa(data_godzina_u[2], buf));
}
void set_min(uint8_t event)
{
	LCD_GoTo(4, 11);

	switch (menu_event)
	{
	case E_LEFT:
		data_godzina_u[3]--;
		break;
	case E_RIGHT:
		data_godzina_u[3]++;
		break;
	}

	LCD_SendText(itoa(data_godzina_u[3], buf));
}

void change_date(uint8_t event)
{
	RTC_t config_data;

	config_data.hour = data_godzina_u[2];
	config_data.mday = data_godzina_u[0];
	config_data.min = data_godzina_u[3];
	config_data.month = data_godzina_u[1];
	config_data.year = rok_u;

	rtc_settime (&config_data);

	sd_log("Zmieniono date/godzine");


	current_menu = 6;
}


void set_period(uint8_t event)
{
	LCD_GoTo(1,10);
	LCD_SendText(itoa(aktualny_czujnik, buf));//wyswietlenie aktualnego czujnika

	LCD_GoTo(2, 11);

	switch (menu_event)
	{
	case E_LEFT:
		meas_period[aktualny_czujnik]--;
		break;
	case E_RIGHT:
		meas_period[aktualny_czujnik]++;
		break;
	}

	LCD_SendText(itoa(meas_period[aktualny_czujnik], buf));
}

void set_meas_number(uint8_t event)
{
	LCD_GoTo(1,10);
	LCD_SendText(itoa(aktualny_czujnik, buf));//wyswietlenie aktualnego czujnika

	LCD_GoTo(3, 11);

	switch (menu_event)
	{
	case E_LEFT:
		meas_number[aktualny_czujnik]--;
		break;
	case E_RIGHT:
		meas_number[aktualny_czujnik]++;
		break;
	}

	LCD_SendText(itoa(meas_number[aktualny_czujnik], buf));
}

void next_or_accept(uint8_t event)
{
	aktualny_czujnik++;
	if (aktualny_czujnik < AMOUNT) //dodanie konfiguracji kolejnego czujnika
	{
		current_menu = 16;
	}
	else //gdy konfiguracje zrobione
	{
		meas_conf konfiguracje[AMOUNT];
		zadanie_pomiarowe zadanie;
		uint8_t i = 0;

		for ( i = 0; i<AMOUNT; i++)
		{
			konfiguracje[i].interwal = meas_period[i];
			konfiguracje[i].numer_czujnika = i;
			konfiguracje[i].liczba_pomiarow = meas_number[i];
			konfiguracje[i].numer_zadania = ktore_zadanie;

			zadanie.konfiguracje_czujnikow[i] = konfiguracje[i];
		}

		zadanie.numer_zadania = ktore_zadanie;

		if (status_SD == SD_ON)
		{
			if ( kolejka_zadan[flaga_kolejki].zakonczone == 0) // wyslanie danych do watku poamirowego
			{
				//(xQueueSend(xTaskQueue, &zadanie, 300)) == errQUEUE_FULL
				LCD_clear();
				LCD_GoTo(1, 2);
				LCD_SendText("Zadania");
				LCD_GoTo(2, 1);
				LCD_SendText("nie dodano!!");
				LCD_GoTo(3, 1);
				LCD_SendText("Kolejka pelna.");
				vTaskDelay(1000/portTICK_RATE_MS);
			}
			else
			{
				kolejka_zadan[flaga_kolejki].zadanie = zadanie; //dodanie zadania do kolejki
				kolejka_zadan[flaga_kolejki].zakonczone = 0; //zadanie przekazane do wykonania

				flaga_kolejki++; //potrzebne do poprawnego zapelniania kolejki

				if (flaga_kolejki > 2)
				{
					flaga_kolejki = 0;
				}

				ktore_zadanie++; //zwiekszenie numeru zadania

				LCD_clear();
				LCD_GoTo(1, 2);
				LCD_SendText("Dodano nowe");
				LCD_GoTo(2, 1);
				LCD_SendText("zadanie");
				LCD_GoTo(3, 1);
				LCD_SendText("do kolejki!!!");
				vTaskDelay(1000/portTICK_RATE_MS);
				vTaskResume(xHandleTaskMeas); //obudzenie watku pomiarowego
			}
		}
		else
		{
			LCD_clear();
			LCD_GoTo(2, 2);
			LCD_SendText("Brak karty SD!");

			vTaskDelay(1000/portTICK_RATE_MS);
		}

		aktualny_czujnik = 0;
		LCD_clear();
		current_menu = 0;
	}
}


void check_queue(uint8_t event)
{
	char buf[4];
	uint8_t i = 0;
	uint8_t licznik_braku = 0; //ile zadan jest pustych

	LCD_clear();

	for (i = 0; i<3; i++)
	{
		if (kolejka_zadan[i].zadanie.numer_zadania != 0)
		{
			if (kolejka_zadan[i].zakonczone == 1)
			{
				LCD_GoTo(i+1, 2);
				LCD_SendText("Zadanie");
				LCD_SendText(itoa(kolejka_zadan[i].zadanie.numer_zadania, buf));
				LCD_SendText(" (t)");
			}
			else
			{
				LCD_GoTo(i+1, 2);
				LCD_SendText("Zadanie");
				LCD_SendText(itoa(kolejka_zadan[i].zadanie.numer_zadania, buf));
				LCD_SendText("    ");
			}
		}
		else
		{
			licznik_braku++;
		}
	}

	if (licznik_braku == 3) //gdy nie ma zadan pomiarowych w kolejce
	{
		LCD_GoTo(2, 2);
		LCD_SendText("Brak zadan!");
	}
}
