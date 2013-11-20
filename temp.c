#include "temp.h"
#include <stddef.h>
#include "stm32f10x.h"
#include "lcd.h"

unsigned char crc8;
unsigned char last_zero; //do wyszukiwania czujnikow
int LastFamilyDiscrepancy; // do wyszukiwania czujnikow
int LastDiscrepancy; //do wyszukiwania czujnikow
uint8_t LastDeviceFlag; // do wyszukiwania czujnikow

// TEST BUILD
static unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};


void uart_config()
{
	//Konfiguracja zegarow
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	//KOnfiguracja DIR
	GPIO_InitTypeDef GPIO_InitStructure1;
	GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure1);
	GPIO_SetBits(GPIOB, GPIO_Pin_8); //ustawienie DIR na 1

	//Kongiguracja P9 jako Tx
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//Konfiguracja PA10 jako Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//Konfiguracja usart
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);

}

int ds_init()
{
	uint8_t tmp[5];

	USART_SendBreak(USART1);
	vTaskDelay(2/portTICK_RATE_MS);

	uart_flush();
	uart_send(0xC1);

	vTaskDelay(2/portTICK_RATE_MS);

	uart_flush();
	uart_send(0x17);
	tmp[0] = uart_receive();
	uart_send(0x45);
	tmp[1] = uart_receive();
	uart_send(0x5B);
	tmp[2] = uart_receive();
	uart_send(0x0F);
	tmp[3] = uart_receive();
	uart_send(0x91);
	tmp[4] = uart_receive();

    if  ((tmp[0] == 0x16) &&
    	 (tmp[1] == 0x44) &&
    	 (tmp[2] == 0x5a) &&
    	 (tmp[3] == 0x00) &&
    	 (tmp[4] == 0x93))
    {
    	return 1; //wszystko dziala
    }

    return 0; //cos poszlo nie tak
}

void uart_flush()
{
	char tmp;
	if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
		tmp = USART_ReceiveData(USART1);

}

void uart_send(uint8_t data)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, data);
}

uint8_t uart_receive()
{
	uint8_t tmp;
	while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
	tmp = USART_ReceiveData(USART1);
	return tmp;
}

int OWReset()
{
	char rslt;

	uart_flush();
	uart_send(0xE3); //command mode
	uart_send(0xC5);

	rslt = uart_receive();

	if(rslt == 0xcd)
		return 1; //sa czujniki na lini
	else
		return 0; // nie ma czujnikow na lini
}


// Bit utility to read and write a bit in the buffer 'buf'.
//
// 'op'    - operation (1) to set and (0) to read
// 'state' - set (1) or clear (0) if operation is write (1)
// 'loc'   - bit number location to read or write
// 'buf'   - pointer to array of bytes that contains the bit
//           to read or write
//
// Returns: 1   if operation is set (1)
//          0/1 state of bit number 'loc' if operation is reading
//
int bitacc(int op, int state, int loc, unsigned char *buf)
{
   int nbyt,nbit;

   nbyt = (loc / 8);
   nbit = loc - (nbyt * 8);

   if (op == 1)
   {
      if (state)
         buf[nbyt] |= (0x01 << nbit);
      else
         buf[nbyt] &= ~(0x01 << nbit);

      return 1;
   }
   else
      return ((buf[nbyt] >> nbit) & 0x01);
}


//konwersja temperatury wybranego czujnika
int OWTempConvert_single(unsigned char* ROM_ID)
{
	unsigned char tmp[30];
	int tmp_len = 0;
	int i = 0;

	//uart_flush();
	uart_flush();
	uart_send(0xE3); //command mode

	uart_send(0x39);
	tmp[tmp_len++] = uart_receive(); //oczekiwane 0x38

	uart_send(0xC1);
	tmp[tmp_len++] = uart_receive(); //oczekiwane 0xcd

	if (tmp[tmp_len - 1] != 0xcd)
	{
		return 0; //brak czujnikow na lini
	}

	uart_send(0xE1); //data mode
	uart_send(0x55); //bedzie podawany ROM
	tmp[tmp_len++] = uart_receive(); //0x55

	for (i = 0; i<8; i++)
	{
		uart_send(ROM_ID[i]);
		tmp[tmp_len++] = uart_receive();
	}

	uart_send(0xe3); //cmd mode
	uart_send(0xef); //strong pullup
	uart_send(0xf1); //terminate pulse
	tmp[tmp_len++] = uart_receive(); //odpowiedz na strong pullup

	uart_send(0xe1); //data mode
	uart_send(0x44); //convert temperature

	//delay_ms(1000);
	vTaskDelay(1000/portTICK_RATE_MS); //oczekiwanie na konwersje

	tmp[tmp_len++] = uart_receive(); //0x44

	uart_send(0xe3); //command mode

	uart_send(0xed); //disarm strong pullup
	uart_send(0xf1); //terminate pulse

	tmp[tmp_len++] = uart_receive(); //??? ef?

	uart_send(0xC5);
	tmp[tmp_len++] = uart_receive(); //oczekiwane 0xcd

	return 1;
}

//konwersja temperatury we wszystkich czujnikach na lini 1-wire
int OWTempConvert()
{
	unsigned char tmp[30];
	int tmp_len = 0;

	//uart_flush();
	uart_flush();
	uart_send(0xE3); //command mode

//
	uart_send(0x39);
	tmp[tmp_len++] = uart_receive(); //oczekiwane 0x38

	uart_send(0xC1);
	tmp[tmp_len++] = uart_receive(); //oczekiwane 0xcd

	uart_send(0xE1); //data mode
	uart_send(0xCC);
	tmp[tmp_len++] = uart_receive();

	uart_send(0xe3); //cmd mode
	uart_send(0xef); //strong pullup
	uart_send(0xf1); //terminate pulse
	tmp[tmp_len++] = uart_receive(); //odpowiedz na strong pullup

	uart_send(0xe1); //data mode
	uart_send(0x44); //convert temperature

	vTaskDelay(1000/portTICK_RATE_MS); //oczekiwanie na konwersje

	tmp[tmp_len++] = uart_receive(); //0x44

	uart_send(0xe3); //command mode

	uart_send(0xed); //disarm strong pullup
	uart_send(0xf1); //terminate pulse

	tmp[tmp_len++] = uart_receive(); //??? ef?

	uart_send(0xC5);
	tmp[tmp_len++] = uart_receive(); //oczekiwane 0xcd

	return 1;
}

int ReadTemp(unsigned char* ROM_ID, uint8_t *temp)
{
	int tmp_len = 0;
	int i = 0;
	unsigned char temperatura[9];
	unsigned char tmp[11];


	uart_send(0xE1); //data mode
	uart_send(0x55); //bedzie podawany ROM
	tmp[tmp_len++] = uart_receive(); //0x55

	for (i = 0; i<8; i++)
	{
		uart_send(ROM_ID[i]);
		tmp[tmp_len++] = uart_receive();
	}

	uart_send(0xBE); //odczyta pamieci

	tmp[tmp_len++] = uart_receive();
	for (i = 0; i<9 ; i++)
	{
		uart_send(0xff);
		temperatura[i] = uart_receive();
	}

    // do dowcrc
    crc8 = 0;
    for (i = 0; i < 9; i++)
       docrc8(temperatura[i]);

    if (crc8 != 0)
    {
    	return 0; //zly odczyt temperatury
    }

	uart_send(0xe3); //command mode
	uart_send(0xc5); //reset
	tmp[tmp_len++] = uart_receive();

	*temp = (uint8_t) temperatura[0]/2;

	if (temperatura[1] != 0)
	{
		*temp = (uint8_t) (-1)*(*temp); //odczytana ujemna temperatura
	}

	return 1; // wszystko ok
}


//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current
// global 'crc8' value.
// Returns current global crc8 value
//
unsigned char docrc8(unsigned char value)
{
   // See Application Note 27

   // TEST BUILD
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}

//odczytanie ROM_ID czujnika
int OWSearch(unsigned char *ROM_ID)
{
	char answer;
	int i = 0;
	unsigned char buf[16]; //przechowanie zwracanych 16 bitow
	unsigned char tmp_rom[8];
	unsigned char send_zero[16]; //tablica 16 zer

	for (i = 0; i<16; i++)
		send_zero[i] = 0x0;

	// if the last call was the last one
	if (LastDeviceFlag)
	{
	   // reset the search
	   LastDiscrepancy = 0;
	   LastDeviceFlag = 0;
	   LastFamilyDiscrepancy = 0;
	   return 0;
	 }

	if(!OWReset())
		return 0; //brak czujnikow

	uart_send(0xE1); //data mode
	uart_send(0xF0); //search cmd
	answer = uart_receive(); //echo of 0xF0
	uart_send(0xE3); //command mode
	uart_send(0xB5); //search accelerator on
	uart_send(0xE1);

	last_zero = 0;
	 // only modify bits if not the first search
	if (LastDiscrepancy != 0)
	{
	   // set the bits in the added buffer
	   for (i = 0; i < 64; i++)
	   {
	      // before last discrepancy
	      if (i < (LastDiscrepancy - 1))
	           bitacc(1, bitacc(0,0,i,&ROM_ID[0]), (short)(i * 2 + 1), &send_zero[0]);

	      // at last discrepancy
	      else if (i == (LastDiscrepancy - 1))
	    	  bitacc(1, 1,(short)(i * 2 + 1), &send_zero[0]);
	         // after last discrepancy so leave zeros
	    }
	 }

	for (i = 0; i<16; i++)
	{
		uart_send(send_zero[i]); // wyslanie 16 zer, wg noty
		buf[i] = uart_receive(); //odbior 16 bajtow
	}


	uart_send(0xE3); //cmd mode
	uart_send(0xa5); //search accelerator off

    for (i = 0; i < 64; i++)
    {
       // get the ROM bit
       bitacc(1, bitacc(0,0,(short)(i * 2 + 1), &buf[0]), i, &tmp_rom[0]);

       // check LastDiscrepancy (dodatek)
       if ((bitacc(0,0,(short)(i * 2),&buf[0]) == 1) &&
           (bitacc(0,0,(short)(i * 2 + 1),&buf[0]) == 0))
       {
          last_zero = i + 1;
          // check LastFamilyDiscrepancy
          if (i < 8)
             LastFamilyDiscrepancy = i + 1;
       }

    }

    // do dowcrc
    crc8 = 0;
    for (i = 0; i < 8; i++)
       docrc8(tmp_rom[i]);

    // check results
    if ((crc8 != 0) || (LastDiscrepancy == 63) || (tmp_rom[0] == 0))
    {
       // error during search
       // reset the search
       LastDiscrepancy = 0;
       LastDeviceFlag = 0;
       LastFamilyDiscrepancy = 0;
       return 0; // blad podczas wyszukiwania czujnikow
    }
    // successful search
    else
    {
       // set the last discrepancy
       LastDiscrepancy = last_zero;

       // check for last device
       if (LastDiscrepancy == 0)
          LastDeviceFlag = 1;

       // copy the ROM to the buffer
       for (i = 0; i < 8; i++)
          ROM_ID[i] = tmp_rom[i];

       return 1; // wszystko ok
    }
}

//odczytanie wszystkich ROM_ID czujnikow
int OWSearch_all(unsigned char ROM_ID[][8])
{
	   LastDiscrepancy = 0;
	   LastDeviceFlag = 0;
	   LastFamilyDiscrepancy = 0;
	   uint8_t sens_num = 0;

	   unsigned char rslt;
	   rslt = OWSearch(ROM_ID[sens_num]);

	   while(rslt)
	   {
		   sens_num++;
		   rslt = OWSearch(ROM_ID[sens_num]);
	   }

	return 1;
}

//=================OPOZNIENIE W ms=========================================
void delay_ms(unsigned int delay)
{
  volatile int i; //zmienna typu volatile
  for(;delay>0;delay--)
    for(i=5000;i>0;i--);
}
