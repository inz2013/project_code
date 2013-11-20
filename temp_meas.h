/*
 * temp_meas.h
 *
 *  Created on: Aug 21, 2013
 *      Author: Piotrus
 */

#include "data_model.h"
#include "SD_card.h"

#ifndef TEMP_MEAS_H_
#define TEMP_MEAS_H_


void vTaskMeasure(void* pvParameters); //funkcja watku mierzenia
void vTaskSensor(void *pvParameters); //funkcja watku pojedynczego cuzjnika


#endif /* TEMP_MEAS_H_ */
