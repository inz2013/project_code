/*
 * menu.h
 *
 *  Created on: Feb 26, 2013
 *      Author: Piotrus
 */
#include <stddef.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "lcd.h"
#include "FRTOS/FreeRTOS.h"
#include "FRTOS/task.h"
#include "FRTOS/queue.h"
#include "FRTOS/portmacro.h"
#include "FRTOS/list.h"
#include "FRTOS/semphr.h"
#include "rtc.h"
#include "data_model.h"


#ifndef MENU_H_
#define MENU_H_

#define E_UP 0
#define E_DOWN 1
#define E_OK 2
#define E_IDLE 3
#define E_RIGHT 4
#define E_LEFT 5




uint8_t card_on;

void vTaskLCD_JOY(void* pvParameters);
void change_menu(const menu_item menu[]);
void display_date10(uint8_t event); //do ogladania daty godziny
void set_year(uint8_t event);
void set_day(uint8_t event);
void set_month(uint8_t event);
void set_hour(uint8_t event);
void set_min(uint8_t event);
void change_date(uint8_t event);
void add_measure_task(uint8_t event);
void check_queue(uint8_t event);
void set_period(uint8_t event);
void set_meas_number(uint8_t event);
void next_or_accept(uint8_t event);

#endif /* MENU_H_ */
