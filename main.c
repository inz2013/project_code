#include <stddef.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "lcd.h"
#include "FRTOS/FreeRTOS.h"
#include "FRTOS/portmacro.h"
#include "FRTOS/list.h"
#include "FRTOS/semphr.h"
#include "rtc.h"
#include "menu.h"
#include "SD_card.h"
#include "temp_meas.h"
#include "temp.h"

static void prvSetupHardware( void );

int main()
{
	prvSetupHardware(); //inicjalizacja

	xLogQueue = xQueueCreate(5, sizeof(xLogMessage));
	xMeasQueue = xQueueCreate(10, sizeof(xMeasMessage));
	xTaskQueue = xQueueCreate(3, sizeof(zadanie_pomiarowe));

	status_pomiarow = MEAS_OFF;
	status_SD = SD_OFF;

	//utworzenie watkow
	xTaskCreate(vTaskLCD_JOY, (signed portCHAR * ) "LCD", configMINIMAL_STACK_SIZE+500, NULL, (tskIDLE_PRIORITY + 1), &xHandleTaskLCD_JOY);
	xTaskCreate(vTaskSD, (signed portCHAR * ) "SD", configMINIMAL_STACK_SIZE + 300, NULL, (tskIDLE_PRIORITY + 2), &xHandleTaskSD);
	xTaskCreate(vTaskMeasure, (signed portCHAR * ) "MEAS", configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 4), &xHandleTaskMeas);

	vTaskStartScheduler();

	return 0;
}

static void prvSetupHardware( void )
{
	eval_init();
	uart_config();

	/* RCC system reset(for debug purpose). */
	 RCC_DeInit();
	  RCC_HSICmd(ENABLE);
	  RCC_AdjustHSICalibrationValue(0);

	    /* Enable HSE. */
		RCC_HSEConfig( RCC_HSE_ON );

		/* Wait till HSE is ready. */
		while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);

	    /* HCLK = SYSCLK. */
		RCC_HCLKConfig( RCC_SYSCLK_Div1 );

	    /* PCLK2  = HCLK. */
		RCC_PCLK2Config( RCC_HCLK_Div1 );

	    /* PCLK1  = HCLK/2. */
		RCC_PCLK1Config( RCC_HCLK_Div2 );

		/* ADCCLK = PCLK2/4. */
		RCC_ADCCLKConfig( RCC_PCLK2_Div4 );

	    /* Flash 2 wait state. */
		*( volatile unsigned long  * )0x40022000 = 0x01;

		/* PLLCLK = 8MHz * 9 = 72 MHz */
		RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_9 );

	    /* Enable PLL. */
		RCC_PLLCmd( ENABLE );

		/* Wait till PLL is ready. */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		/* Select PLL as system clock source. */
		RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source. */
		while (RCC_GetSYSCLKSource() != 0x08);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,  ENABLE );
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);//wlaczenie zegara portu joystika
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);


		NVIC_InitTypeDef NVIC_InitStruct;
		EXTI_InitTypeDef EXTI_InitStruct;

		/* Set the Vector Table base address at 0x08000000. */
		NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

		 //incjalizacja przerwania

		NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

		NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);

		GPIO_InitTypeDef  GPIO_InitStructure;

		GPIO_InitStructure.GPIO_Pin = JOY_UP | JOY_DOWN | JOY_L | JOY_R; //piny które chcemy skonfigurowaæ
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//tryb wyjœæ portu - wyjœcie push-pull
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 	//szybkoœæ wyjœcia
		GPIO_Init(JOY_PORT, &GPIO_InitStructure); //inicjalizacja portu

		GPIO_InitStructure.GPIO_Pin = JOY_OK; //piny które chcemy skonfigurowaæ
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//tryb wyjœæ portu - wyjœcie push-pull
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 	//szybkoœæ wyjœcia
		GPIO_Init(LCD_PORT, &GPIO_InitStructure); //GPIOB

		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource0);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

		EXTI_InitStruct.EXTI_Line = EXTI_Line0;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStruct);

		EXTI_InitStruct.EXTI_Line = EXTI_Line1;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStruct);

		EXTI_InitStruct.EXTI_Line = EXTI_Line2;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStruct);

		EXTI_InitStruct.EXTI_Line = EXTI_Line3;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStruct);

		EXTI_InitStruct.EXTI_Line = EXTI_Line5;
		EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
		EXTI_InitStruct.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStruct);

		/* Configure HCLK clock as SysTick clock source. */
		SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//tryb wyjœæ portu - wyjœcie push-pull
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 	//szybkoœæ wyjœcia
		GPIO_Init(GPIOB, &GPIO_InitStructure); //inicjalizacja portu

		GPIO_SetBits(GPIOB,  GPIO_Pin_2 | GPIO_Pin_1);
}
