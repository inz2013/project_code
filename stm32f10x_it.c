/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    10/15/2010
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "FRTOS/FreeRTOS.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)
//{
//}
//
///**
//  * @brief  This function handles Debug Monitor exception.
//  * @param  None
//  * @retval None
//  */
//void DebugMon_Handler(void)
//{
//}
//
///**
//  * @brief  This function handles PendSVC exception.
//  * @param  None
//  * @retval None
//  */
//void PendSV_Handler(void)
//{
//}
//
///**
//  * @brief  This function handles SysTick Handler.
//  * @param  None
//  * @retval None
//  */
//void SysTick_Handler(void)
//{
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


//MOJE DOPISKI
uint8_t licznik1 = 0,
		licznik2 = 0,
		licznik3 = 0,
		licznik4 = 0,
		licznik5 = 0;

void EXTI0_IRQHandler(void){ //dla klawisza UP

	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	//uint8_t licznik = 0;

	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		if (licznik1 == 200)
		{
			licznik1 = 0;
			xSemaphoreGiveFromISR(xSemaphoreUP, &xHigherPriorityTaskWoken);
			EXTI_ClearITPendingBit(EXTI_Line0);
		}
		else
		{
			licznik1++;
		}
	}
}

void EXTI1_IRQHandler(void){ //dla klawisza DOWN

	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	//uint8_t licznik = 0;

	if(EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		if (licznik2 == 200)
		{
			licznik2 = 0;
			xSemaphoreGiveFromISR(xSemaphoreDOWN, &xHigherPriorityTaskWoken);
			EXTI_ClearITPendingBit(EXTI_Line1);
		}
		else
		{
			licznik2++;
		}
	}
}

void EXTI2_IRQHandler(void){ //dla klawisza DOWN

	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	//uint8_t licznik = 0;

	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		if (licznik3 == 200)
		{
			licznik3 = 0;
			xSemaphoreGiveFromISR(xSemaphoreLEFT, &xHigherPriorityTaskWoken);
			EXTI_ClearITPendingBit(EXTI_Line2);
		}
		else
		{
			licznik3++;
		}

	}
}

void EXTI3_IRQHandler(void){ //dla klawisza DOWN

	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	//uint8_t licznik = 0;

	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		if (licznik4 == 200)
		{
			licznik4 = 0;
			xSemaphoreGiveFromISR(xSemaphoreRIGHT, &xHigherPriorityTaskWoken);
			EXTI_ClearITPendingBit(EXTI_Line3);
		}
		else
		{
			licznik4++;
		}
	}
}

void EXTI9_5_IRQHandler(void){

	static portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	//uint8_t licznik = 0;

	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		if (licznik5 == 200)
		{
			licznik5 = 0;
			xSemaphoreGiveFromISR(xSemaphoreOK, &xHigherPriorityTaskWoken);
			EXTI_ClearITPendingBit(EXTI_Line5);
		}
		else
		{
			licznik5++;
		}
	}
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
