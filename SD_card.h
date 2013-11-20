/*
 * SD_card.h
 *
 *  Created on: Apr 16, 2013
 *      Author: Piotrus
 */
#include "rtc.h"
#include <stddef.h>
#include "stm32f10x.h"
#include "data_model.h"
#include "SecureDigital/ffconf.h"
#include "SecureDigital/ff.h"

#ifndef SD_CARD_H_

#define SD_CARD_H_

void vTaskSD(void* pvParameters);
void sd_log(char* log_msg);

FRESULT create_path(FIL* plik, RTC_t date, uint8_t zadanie, uint8_t numer_czujnika, char* buf); //funkcja towrzy sciezke i plik do zapisu stanu wynikow pomiarow


#endif /* SD_CARD_H_ */
