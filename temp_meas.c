#include "temp_meas.h"
#include "temp.h"
#include "FRTOS/semphr.h"
#include "FRTOS/task.h"

uint8_t flaga_zadan = 0; //do ustalenia ktore zadanie jest teraz brane pod uwage

void vTaskMeasure(void* pvParameters)
{
	END_FLAG = 2;
	RTC_t czas;
	//ustawienie poczatkowej daty godziny

	czas.year = 2013;
	czas.month = 11;
	czas.mday = 11;
	czas.wday = 5;
	czas.hour = 12;
	czas.min = 18;
	czas.sec = 0;
	czas.dst = 0;
	rtc_settime (&czas);
	///koniec ustawiania daty godziny

	zadanie_pomiarowe zadanie; //do odebrania z kolejki
	uint8_t i = 0;

	for (i = 0; i<3; i++)
	{
		kolejka_zadan[i].zakonczone = 1;
		kolejka_zadan[i].zadanie.numer_zadania = 0;
	}

	portBASE_TYPE stan;

	if (!ds_init()) //inicjalizacja DS2480b
	{
		//sd_log("Inicjalizacja DS2480b NIE powiodla sie!!!!");
	}
	else
	{
		//sd_log("Inicjalizacja DS2480b powiodla sie!!!!");
	}

	if (!OWSearch_all(ROM_ID))
	{
		//sd_log("Nie udalo sie znalezc czujnikow");
	}

	while (1)
	{
		//xSemaphoreTake(xSemaphoreMeasQueue, 100); //wejscie do sekcji krytycznej
		//odebranie zadnaia pomiarowego z kolejki
		if ( (kolejka_zadan[flaga_zadan].zakonczone != 1) && (END_FLAG == AMOUNT))
		{
			if (flaga_zadan > 2)
			{
				flaga_zadan = 0;
			}

			zadanie = kolejka_zadan[flaga_zadan].zadanie;
			kolejka_zadan[flaga_zadan].zakonczone = 1; //zadanie trwa

//			for (i = 0; i<AMOUNT; i++)
//			{
//				vTaskDelete(xHandleSensor[i]);
//			}

			vTaskDelay(5000/portTICK_RATE_MS); //oczekiwanie na porzadek po poprzednich zadaniach

		//	sd_log("Rozpoczeto nowe zadanie pomiarowe");
			//status_pomiarow = MEAS_ON;
			END_FLAG = 0;

			for (i = 0; i<AMOUNT; i++)
			{
			//	sd_log("Tworzenie taska");
				stan = xTaskCreate(vTaskSensor, (signed portCHAR * ) "Sensor", configMINIMAL_STACK_SIZE, (void*) &zadanie.konfiguracje_czujnikow[i], (tskIDLE_PRIORITY + 3), &xHandleSensor[i]);

				if (stan != pdPASS)
				{
				//	sd_log("nie mozna utworzyc taska");
					GPIO_ResetBits(GPIOB, GPIO_Pin_2); //zapalenie LEDa, informacja DEBUG, nie ma pamieci na kolejne zadnanie
				}
			}
		}
		else
		{
			//xSemaphoreGive(xSemaphoreMeasQueue); //wyjscie z sekcji krytycznej
			vTaskSuspend( NULL ); //uspienie zadania jesli brak zadan w kolejce, albo trwaja pomiary
		}

		//xSemaphoreGive(xSemaphoreMeasQueue); //wyjscie z sekcji krytycznej
	}
}

//funkcja zadnia pojedynczego czujnika
void vTaskSensor(void *pvParameters)
{
	meas_conf *zadanie_czujnika = (meas_conf*) pvParameters;
	uint8_t i = 0;
	RTC_t czas;
	xMeasMessage pomiar;
	uint8_t wynik_pomiaru = 0;
	portTickType lastTime = 0;

//	sd_log("Jestem w funkcji czujnika");

	for (i = 0; i < zadanie_czujnika->liczba_pomiarow; i++)
	{
		status_pomiarow = MEAS_ON;

		xSemaphoreTake(xSemaphoreMeas, portMAX_DELAY); //wejscie do sekcji krytycznej

		rtc_gettime(&czas); //pobranie aktualnego czasu
		lastTime = xTaskGetTickCount();

		OWTempConvert_single(ROM_ID[zadanie_czujnika->numer_czujnika]);
		ReadTemp(ROM_ID[zadanie_czujnika->numer_czujnika], &wynik_pomiaru);
		pomiar.pomiar = wynik_pomiaru;
		pomiar.czas_pomiaru = czas;
		pomiar.numer_zadania = zadanie_czujnika->numer_zadania;
		pomiar.numer_czujnika = zadanie_czujnika->numer_czujnika;

		vTaskResume(xHandleTaskSD);

		if (xQueueSend(xMeasQueue, &pomiar, 300) == errQUEUE_FULL) //wyslanie wyniku pomiaru
		{

		}

		xSemaphoreGive(xSemaphoreMeas); //wyjscie z sekcji krytycznej

		vTaskDelayUntil(&lastTime, (zadanie_czujnika->interwal)*(1000/portTICK_RATE_MS));
	}

	//sd_log("Czujnik zakonczyl prace");
	END_FLAG++;
	if (END_FLAG == AMOUNT)
	{
		status_pomiarow = MEAS_OFF; //koniec zadania pomiarowego
		kolejka_zadan[flaga_zadan].zadanie.numer_zadania = 0; //zadanie zakonczone
		flaga_zadan++; //kolejne miejsce w kolejce
	}
	vTaskResume(xHandleTaskMeas); //wznowienie procesu pomiarowego

	vTaskDelete(NULL);
}
