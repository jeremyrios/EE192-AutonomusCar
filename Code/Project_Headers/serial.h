/********************************************/
/* 	Serial Communication					*/
/* 	                                        */
/*	                                        */
/********************************************/

#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "utilities.h"
#include "MPC5604B.h"

#define IN_BUFFER_SIZE 16
#define OUT_BUFFER_SIZE 150

typedef struct
{
	fifo_t fifo;
	char buffer[OUT_BUFFER_SIZE];
	uint8_t length;	
}buffer_t;

#define TRANSMISSION_DONE()	(1 == LINFLEX_0.UARTSR.B.DTF)
#define RECEPTION_DONE()	(1 == LINFLEX_0.UARTSR.B.DRF)

uint8_t TransmitCharacter(uint8_t ch);
void initSerial();
void Rx();
void Tx();
void init_LinFLEX_0_UART(void);

buffer_t* getInBuffer();
buffer_t* getOutBuffer();

#endif
