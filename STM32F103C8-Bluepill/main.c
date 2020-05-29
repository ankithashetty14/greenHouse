/*************************************************************************
Read temperature, light intensity and soil moisture level send over UART

UART Connections:          PB10 - TX
									         PB11 - RX
LM35 Temperature Sensor: 	 PA0
Ambient Light Sensor(LDR): PA1
Soil Moisture Sensor: 	   PA2
**************************************************************************/
#include <string.h>
#include <stdio.h>
#include "stm32f10x.h"

uint32_t __IO time_us, time_ms;
void delayInit(void);
void delay_us(uint32_t us_value);
void delay_ms(uint32_t ms_value);

void UARTInit(void);
void SendData(uint8_t *pTxBuffer, uint32_t Len);

void ADCInit(void);
float temp1;
float temp2;
float temp3;
char temperature[5];
char lightIntensity[5];
char soilMoisture[5];

void DMAInit(void);
uint16_t samples[3] = {0, 0, 0};

int main(void)
{
	delayInit();				// Initialize delay function
	UARTInit();
	DMAInit();
	ADCInit();
	
	while (1)
	{
		temp1 = (samples[0]*330)/4096;
		sprintf(temperature, "%d", (int)temp1);
		SendData((uint8_t*)"Temp:", 5);
		SendData((uint8_t*)temperature, strlen(temperature));
		SendData((uint8_t*)"\n", 1);
		delay_ms(2000);
		
		temp2 = ((float)samples[1]/4096.00)*100.00;
		sprintf(lightIntensity, "%d", (int)temp2);
		SendData((uint8_t*)"Lite:", 5);
		SendData((uint8_t*)lightIntensity, strlen(lightIntensity));
		SendData((uint8_t*)"\n", 1);
		delay_ms(2000);
		
		temp3 = ((float)samples[2]/4096.00)*100.00;
		sprintf(soilMoisture, "%d", (int)temp3);
		SendData((uint8_t*)"Mois:", 5);
		SendData((uint8_t*)soilMoisture, strlen(soilMoisture));
		SendData((uint8_t*)"\n", 1);
		delay_ms(2000);
	}
}

/********************************** Delay Functions********************************/
// SysTick Timer Initialization
void delayInit(void)
{
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000000);	// Configure timer to run at 1MHz
}

// Generate delay in microseconds
void delay_us(uint32_t us_value)
{
	time_us = us_value;
	while(time_us);
}

// Generate delay in miliseconds
void delay_ms(uint32_t ms_value)
{
		time_ms = ms_value;
		while(time_ms--)
		{
			delay_us(1000);
		}
}

// Handler for SysTick Interrupt
void SysTick_Handler()
{
	if(time_us)
	{
		time_us--;
	}
}

/****************************** UART Functions ********************************/
// UART Configuration
void UARTInit(void)
{
	RCC->APB1ENR |= 0x00040000;		// Enable clock for UART3
	RCC->APB2ENR |= 0x0008;				// Enable clock for GPIOB
	GPIOB->CRH 	 |= 0x0900;     	// Configure PA10 as TX
	GPIOB->CRH   |= 0x4000;     	// Configure PA11 as RX
	USART3->CR1  |= 0x2000;				// Enable USART Peripheral
	USART3->CR1  |= 0x0004;				// Enable USART Receiver
	USART3->CR1  |= 0x0008;				// Enable USART Transmitter
	USART3->BRR   = 0x0EA6;				// Set baud rate to 9600
}

// UART Send Data
void SendData(uint8_t *pTxBuffer, uint32_t Len)
{
	uint8_t i;
	for(i=0; i<Len; i++)
	{
		while(!(USART3->SR & 0x80));		// wait until TXE flag is set in the SR
		USART3->DR = *pTxBuffer;
		pTxBuffer++;
	}
	while(!(USART3->SR & 0x40));
}

/************************* ADC Functions ***********************************/

// Configure ADC
void ADCInit(void)
{
	RCC->APB2ENR |= 0x0004; 			// Enable clock for GPIOA
	RCC->APB2ENR |= 0x0001; 			// Enable clock for AFIO
	RCC->APB2ENR |= 0x0200; 			// Enable clock for ADC1
	RCC->CFGR    |= 0x8000;    		// Set ADC clock to 12MHz
	
	GPIOA->CRL   &= ~0x000F;   		// Set PA0 as analog input
	GPIOA->CRL   &= ~0x00F0;   		// Set PA1 as analog input
	GPIOA->CRL   &= ~0x0F00;   		// Set PA2 as analog input
	
	ADC1->SMPR2  |= (7 << 0);			// Set sampling rate for PA0
	ADC1->SMPR2  |= (7 << 3);			// Set sampling rate for PA1
	ADC1->SMPR2  |= (7 << 6);			// Set sampling rate for PA2
	
	ADC1->SQR1   |= (2 << 20); 		// Three channels in sequence
	ADC1->SQR3   |= (0 << 0);			// Channel 0 is first in sequence
	ADC1->SQR3   |= (1 << 5);			// Channel 1 is second in sequence
	ADC1->SQR3   |= (2 << 10);		// Channel 2 is third in sequence
	
	ADC1->CR1    |= 0x0100; 			// Turn on scan mode
	ADC1->CR2    |= 0x0100;				// Enable DMA mode
	
	ADC1->CR2    |= 0x0003;       // Turn on ADC 
	delay_ms(1);									// Wait for ADC to stabilize
	ADC1->CR2    |= 0x0001;       // Turn on ADC 
	ADC1->CR2    |= 0x0004;       // Turn on Calibration
	delay_ms(2);
}

/******************************** DMA Functions *********************************/
// DMA Initialization
void DMAInit(void)
{
	RCC->AHBENR |= 0x0001;													// Enable clock for DMA 1
	DMA1_Channel1->CPAR = (uint32_t)(&(ADC1->DR));  // Peripheral data register address
	DMA1_Channel1->CMAR = (uint32_t)(samples);			// Memory Address
	DMA1_Channel1->CNDTR = 3;												// Number of transfers
	DMA1_Channel1->CCR |= 0x00A0;		         			  // Enable circular and increment mode
	DMA1_Channel1->CCR |= 0x0500;										// Set data size
	DMA1_Channel1->CCR |= 0x0001;   								// Enable DMA
}
