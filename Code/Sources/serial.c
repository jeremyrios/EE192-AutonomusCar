/********************************************/
/* 	Serial Communication					*/
/* 	                                        */
/*	                                        */
/********************************************/

#include "serial.h"

buffer_t inBuffer;
buffer_t outBuffer;

buffer_t* getInBuffer()
{
	return &inBuffer;
}


buffer_t* getOutBuffer()
{
	return &outBuffer;
}


uint8_t TransmitCharacter(uint8_t ch)
{

	LINFLEX_0.BDRL.B.DATA0 = ch;  			/* write character to transmit buffer */
	
	return 1;
}
	

void init_LinFLEX_0_UART (void) 
{	

	/* enter INIT mode */
	LINFLEX_0.LINCR1.R = 0x0081; 		/* SLEEP=0, INIT=1 */
	
	/* wait for the INIT mode */
	while (0x1000 != (LINFLEX_0.LINSR.R & 0xF000)) {}
		
	/* configure pads */
	SIU.PCR[18].R = 0x0604;     		/* Configure pad PB2 for AF1 func: LIN0TX */
	SIU.PCR[19].R = 0x0100;     		/* Configure pad PB3 for LIN0RX */	
	
	/* configure for UART mode */
	LINFLEX_0.UARTCR.R = 0x0001; 		/* set the UART bit first to be able to write the other bits */
	LINFLEX_0.UARTCR.R = 0x0033; 		/* 8bit data, no parity, Tx and Rx enabled, UART mode */
								 		/* Transmit buffer size = 1 (TDFL = 0) */
								 		/* Receive buffer size = 1 (RDFL = 0) */
	
	/* configure baudrate 9600 */
	/* assuming 64 MHz peripheral set 1 clock */		
	LINFLEX_0.LINFBRR.R = 11;
	LINFLEX_0.LINIBRR.R = 416;
		
	/* enter NORMAL mode */
	LINFLEX_0.LINCR1.R = 0x0080; 		/* INIT=0 */	
}


void Tx()
{
	uint8_t ch;
	uint8_t bytes = (uint8_t)fifo_read(&outBuffer.fifo,&ch,1);
	outBuffer.length -= bytes;
	LINFLEX_0.UARTSR.R = 0x0002; 		/* clear the DTF flag and not the other flags */
	if(bytes>0)
		TransmitCharacter(ch);
	
}


void Rx()
{

	if (1 == LINFLEX_0.UARTSR.B.RMB) 
	{  /* Check for Release Message Buffer */

		uint8_t ch = (uint8_t)LINFLEX_0.BDRM.B.DATA4;
		/* get the data */
		fifo_write(&inBuffer.fifo,(const void*)(&ch),sizeof(uint8_t));
		inBuffer.length++;
	}

		
	/* clear the DRF and RMB flags by writing 1 to them */
	LINFLEX_0.UARTSR.R = 0x0204;	

}


void initSerial()
{
	inBuffer.length = 0;
	outBuffer.length = 0;
	fifo_init(&inBuffer.fifo,inBuffer.buffer,IN_BUFFER_SIZE);
	fifo_init(&outBuffer.fifo,outBuffer.buffer,OUT_BUFFER_SIZE);
	init_LinFLEX_0_UART ();	
	TransmitCharacter('\0');
}