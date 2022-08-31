/*
 * ESP_program.c
 *
 *  Created on: Aug 28, 2022
 *      Author: Sirito
 */

/** INCLUDES **/
// HAL
// Libraries
#include "STD_TYPES.h"
#include "BIT_MATH.h"
// Interface of lower layer (MCAL)
#include "USART/USART_interface.h"
#include "SYSTICK/SYSTICK_interface.h"
#include "NVIC/NVIC_interface.h"
#include "GPIO/GPIO_interface.h"
#include "RCC/RCC_interface.h"
// Own driver files
#include <ESP_interface.h>
#include <ESP_config.h>
#include <ESP_private.h>
#include <string.h>
volatile u8 array[1000];
/** GLOBAL **/
u8 ReceiveArray[100] = { 0 };
u32 arr_size = 0;
u8 ReceivedByte;
#define FALSE 0
#define TRUE 1
u8 StringMatch(u8* SourceString, u8* StringToFind){
    u8 state = FALSE;
    for (u8 i = 0; SourceString[i] != '\0'; i++){
        if (SourceString[i] == StringToFind[0]) {
            u8 j = 1;
            for (; StringToFind[j] != '\0'; j++){
                i++;
                if (StringToFind[j] != SourceString[i])
                    break;
            }
            if (StringToFind[j] == '\0') {
                state = TRUE;
                break;
            }
        }
    }

    return state;
}


/** FUNCTIONS **/
void ESP_Initilization(void) {
	/** SYSTICK **/
	// Initialize SYSTICK
	STK_voidInit(SYSTICK_AHB_8);

	/** GPIO **/
	// Enable GPIOA Clock
	RCC_void_PeripheralClockEnable(RCC_APB2, RCC_GPIOA);
	// Enable USART1 Clock
	RCC_void_PeripheralClockEnable(RCC_APB2, RCC_USART1);
	// Pin A9: USART1 Tx
	GPIO_void_SetPinMode(GPIO_PORTA,PIN9,GPIO_OUTPUT_ALT_PP_2MHZ);
	// Pin A10: USART1 Rx
	GPIO_void_SetPinMode(GPIO_PORTA,PIN10,GPIO_INPUT_FLOATING);

	/** USART **/
	// Set Baud Rate to 115200 BPS
	USART_SetBaudRate(BAUD_RATE_115200);
	// Enable USART, Tx and Rx
	USART_EnableUSART(USART_1);
	// Enable USART1 Interrupt
	NVIC_voidEnableInterrupt(NVIC_USART1);
	USART1_ReceiveInterruptEnable(USART_1);

	/** ESP **/
	// Echo Mode
	USART_SendString(USART_1,(u8 *) EchoOff);
	// Poll untill you receive the correct response for this command
	//while(!(ReceiveArray[0]=='\r' && ReceiveArray[1]=='\n' && ReceiveArray[2]=='O' && ReceiveArray[3]=='K' && ReceiveArray[4] =='\r' &&ReceiveArray[5]=='\n')){};
	//while(!StringMatch(ReceiveArray,(u8 *)"OK\r\n")){};

	while(!StringMatch(ReceiveArray,(u8 *)"OK\r\n") ){
		if(StringMatch(ReceiveArray,(u8 *)"FAIL")){
			break;
		}
	}
	if(StringMatch(ReceiveArray,(u8 *)"FAIL")){
		arr_size=0;
		for(u8 i=0;i<50;i++)
			ReceiveArray[i]=0;
		ESP_Initilization();
	}
	// Restart Received UART data array size for the next command to fill it
	arr_size=0;
	for(u8 i=0;i<10;i++)
		ReceiveArray[i]=0;

	// Check WiFi Mode
	// 1 : station mode (Connect to router)
	// 2 : softAP mode (Access point)
	// 3 : softAP + station mode (Both)
	USART_SendString(USART_1, (u8 *)WifiMode1);
	// Poll untill you receive the correct response for this command
	//while(!(ReceiveArray[0]=='\r' && ReceiveArray[1]=='\n' && ReceiveArray[2]=='O' && ReceiveArray[3]=='K' && ReceiveArray[4] =='\r' &&ReceiveArray[5]=='\n')){};
	//while(!StringMatch(ReceiveArray,(u8 *)"OK\r\n")){};
	while(!StringMatch(ReceiveArray,(u8 *)"OK\r\n") ){
		if(StringMatch(ReceiveArray,(u8 *)"FAIL")){
			break;
		}
	}
	if(StringMatch(ReceiveArray,(u8 *)"FAIL")){
		arr_size=0;
		for(u8 i=0;i<50;i++)
			ReceiveArray[i]=0;
		ESP_Initilization();
	}


	// Restart Received UART data array size for the next command to fill it
	arr_size=0;
	for(u8 i=0;i<10;i++)
		ReceiveArray[i]=0;

}

void ESP_ConnectToWIFI(u8* SSID, u8* password) {
	USART_SendString(USART_1, (u8 *) "AT+CWJAP_DEF=\"");
	USART_SendString(USART_1, SSID);
	USART_SendString(USART_1, (u8 *) "\",\"");
	USART_SendString(USART_1, password);
	USART_SendString(USART_1, (u8 *) "\"\r\n");
	//while(!(ReceiveArray[48]=='O' && ReceiveArray[49]=='K' && ReceiveArray[51]=='\n')){};
	while(!StringMatch(ReceiveArray,(u8 *)"OK\r\n") ){
		if(StringMatch(ReceiveArray,(u8 *)"FAIL")){
			break;
		}
	}
	if(StringMatch(ReceiveArray,(u8 *)"FAIL")){
		arr_size=0;
		for(u8 i=0;i<50;i++)
			ReceiveArray[i]=0;
		ESP_ConnectToWIFI(SSID,  password);
	}
	// Restart Received UART data array size for the next command to fill it	arr_size=0;
	arr_size=0;
	for(u8 i=0;i<50;i++)
		ReceiveArray[i]=0;
}

void ESP_ConnectToSite(u8* IP) {
	USART_SendString(USART_1, (u8 *) "AT+CIPSTART=\"TCP\",\"");
	USART_SendString(USART_1, IP);
	USART_SendString(USART_1, (u8 *) "\",");
	USART_SendString(USART_1, (u8 *) "80");
	USART_SendString(USART_1, (u8 *) "\r\n");
	//STK_DelayMS(100);

	//while(!(ReceiveArray[11]=='O' && ReceiveArray[12]=='K' && ReceiveArray[14]=='\n')){};
	while(!StringMatch(ReceiveArray,(u8 *)"OK\r\n"));

	arr_size=0;
	for(u8 i=0;i<50;i++)
		ReceiveArray[i]=0;
}

u8 ESP_SendData(u8* DataLength, u8* Data){
	// Send Data
	USART_SendString(USART_1, (u8 *) "AT+CIPSEND=");
	USART_SendString(USART_1, DataLength);
	USART_SendString(USART_1, (u8 *) "\r\n");
	//while(!(ReceiveArray[6]=='>'&&ReceiveArray[7]==' ')){};
	while(!StringMatch(ReceiveArray,(u8 *)"> "));
	arr_size=0;
	for(u8 i=0;i<50;i++)
		ReceiveArray[i]=0;
	USART_SendString(USART_1, Data);
	USART_SendString(USART_1, (u8 *) "\r\n");
	//while(!(ReceiveArray[38]=='C' &&ReceiveArray[45]=='\n')){};
	while(!StringMatch(ReceiveArray,(u8 *)"CLOSED\r\n"));

	u8 data= ReceiveArray[37];
	arr_size=0;
	for(u8 i=0;i<55;i++)
		ReceiveArray[i]=0;

	return data;
}
/** INTERRUPTS HANDLING **/
// This interrupt is triggered each time RXNE is 1 meaning that "Received data is ready to be read"
void USART1_IRQHandler(void) {
	ReceivedByte = USART_ReceiveByte(USART_1);
	ReceiveArray[arr_size] = ReceivedByte;
	arr_size++;
}
