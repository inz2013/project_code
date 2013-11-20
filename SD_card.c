#include "SD_card.h"
#include "menu.h"


char sciezka_pliku[46];

void vTaskSD(void* pvParameters)
{
	FATFS FileSystemObject;

	FRESULT fresult; //do psrawdzania poprawnosci wykonania funkcji
	FIL plik_meas; // uchwyt do pliku pomiaru
		//plik_log; //uchwyt do pliku logu
	unsigned int ilosc_bajtow;
	xLogMessage log_message; // informacja do loga
	xMeasMessage meas_message; //wynik pomiaru

	card_on = 0;

	fresult = f_mount(0, &FileSystemObject);
	fresult = f_mkdir("POMIARY");

	if (fresult == FR_OK || fresult == FR_EXIST)
		status_SD = SD_ON;

	while(1)
	{
		//najistotniejsze - zapis wyniku pomiaru
        if (xQueueReceive( xMeasQueue, &meas_message, 0) != errQUEUE_EMPTY)
		{

			fresult = create_path(&plik_meas, meas_message.czas_pomiaru, meas_message.numer_zadania, meas_message.numer_czujnika, sciezka_pliku);
			fresult = f_lseek(&plik_meas, f_size(&plik_meas)); //dopisywanie do konca pliku

			if (fresult != FR_OK)
				status_SD = SD_OFF;
			else
				status_SD = SD_ON;

			ilosc_bajtow = f_printf(&plik_meas, "%d-%d-%d, %d:%d:%d \t %d\n", meas_message.czas_pomiaru.mday,
														  	 meas_message.czas_pomiaru.month,
														  	 meas_message.czas_pomiaru.year,
														  	 meas_message.czas_pomiaru.hour,
														  	 meas_message.czas_pomiaru.min,
														  	 meas_message.czas_pomiaru.sec,
														  	 meas_message.pomiar);
			fresult = f_close(&plik_meas);
		}
//		//dopisanie informacji do loga
//        else if (xQueueReceive( xLogQueue, &log_message, 0) != errQUEUE_EMPTY)
//		{
//			fresult = f_open(&plik_log, "POMIARY/log.txt", FA_OPEN_ALWAYS | FA_WRITE );
//			fresult = f_lseek(&plik_log, f_size(&plik_log)); //dopisywanie do konca pliku
//
//			if (fresult != FR_OK)
//				status_SD = SD_OFF;
//			else
//				status_SD = SD_ON;
//
//			ilosc_bajtow = f_printf(&plik_log, "%d-%d %d:%d:%d \t %s\n", log_message.czas.mday, log_message.czas.month, log_message.czas.hour, log_message.czas.min, log_message.czas.sec, log_message.logMessage);
//
//			fresult = f_close(&plik_log);
//		}



		if (xQueueReceive( xLogQueue, &log_message, 0) == errQUEUE_EMPTY
			&& xQueueReceive( xMeasQueue, &meas_message, 0) == errQUEUE_EMPTY)
		{
			vTaskSuspend( NULL ); //uspienie taska do zapisu
		}

	}
}

//rozmiar informacji do loga koniecznie 25 znakow
void sd_log(char* log_msg)
{
	xLogMessage message;

	message.logMessage = log_msg;
	rtc_gettime(&message.czas);
	xQueueSend( xLogQueue, &message, portMAX_DELAY);
	vTaskResume(xHandleTaskSD); //obudzenie taska do zapisu
}

FRESULT create_path(FIL* plik, RTC_t date, uint8_t zadanie, uint8_t numer_czujnika, char* buf)
{
	char bufor[5];
	uint8_t i = 0;
	char poczatek[]= "POMIARY/";
	char zadanie_tab[] = "Zadanie";
	char czujnik[] = "Czujnik";
	char rozszerzenie[] = ".txt";
	FRESULT fresult; //do psrawdzania poprawnosci wykonania funkcji

	fresult = f_mkdir("POMIARY");

	for(i=0; i<8 ; i++)
	{
		buf[i] = poczatek[i];
	}

	char* dzien = itoa(date.mday, bufor);
	if (date.mday > 9)
	{
		buf[8] = dzien[0];
		buf[9] = dzien[1];
	}
	else
	{
		buf[8] = '0';
		buf[9] = dzien[0];
	}

	buf[10] = '_';

	char* miesiac = itoa(date.month, bufor);
	if (date.month < 10)
	{
		buf[11] = '0';
		buf[12] = miesiac[0];
	}
	else
	{
		buf[11] = miesiac[0];
		buf[12] = miesiac[1];
	}

	buf[13] = '_';
	char* rok = itoa(date.year, bufor);
	buf[14] = rok[2];
	buf[15] = rok[3];
	buf[16] = '\0';

	fresult = f_mkdir(buf); //stworzenie pierwszego podfolderu

	buf[16] = '/';

	for (i=17; i<24 ; i++ )
	{
		buf[i] = zadanie_tab[i-17];
	}

	char* zadanie_t = itoa(zadanie, bufor);

	if (zadanie < 10)
	{
		buf[24] = '0';
		buf[25] = zadanie_t[0];
	}
	else
	{
		buf[24] = zadanie_t[0];
		buf[25] = zadanie_t[1];
	}
	buf[26] = '\0';

	f_mkdir(buf); // stworzenie drugiego podfolderu

	buf[26] = '/';

	for (i=27; i<34; i++)
	{
		buf[i] = czujnik[i-27];
	}

	char* numer_czujnika_t = itoa(numer_czujnika, bufor);

	if (numer_czujnika < 10)
	{
		buf[34] = '0';
		buf[35] = numer_czujnika_t[0];
	}
	else
	{
		buf[34] = numer_czujnika_t[0];
		buf[35] = numer_czujnika_t[1];
	}

	for (i = 36; i < 41 ; i++)
	{
		buf[i] = rozszerzenie[i - 36];
	}

	fresult = f_open(plik, buf, FA_OPEN_ALWAYS | FA_WRITE);

	return fresult;
}





