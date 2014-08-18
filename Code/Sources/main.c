/* main.c - Hardware vector mode program using C isr */
/* Feb 12, 2009 S.Mihalik Initial version based on previous AN2865 example */
/* May 22 2009 S. Mihalik- disabled watchdog, simplified mode, clock code */
/* Jul 03 2009 S Mihalik - Simplified code */
/* Mar 14 2010 SM - modified initModesAndClock, updated header file */
/* Copyright Freescale Semiconductor, Inc. 2009. All rights reserved. */

#include "MPC5604B.h"
#include "utilities.h"
#include "protocol.h"
#include "drive.h"
#include "serial.h"
#include "camera.h"
#include "control.h"
#include "IntcInterrupts.h"
#include "Exceptions.h"
#include <cmath>
#include <cstring>
#include <cstdio>

#define MESSAGE(str)    strcpy(msg,str); msgLength=strlen(msg)


buffer_t* in;
buffer_t* out;


uint32_t Pit1Ctr = 0;   /* Counter for PIT 1 interrupts */
uint32_t SWirq4Ctr = 0; /* Counter for software interrupt 4 */
uint32_t clock;
uint32_t A,B;
int8_t flag_lineDone = 0;
uint8_t prevEnc;

extern volatile uint16_t Result[128];
extern volatile uint8_t  lineADC[128];
extern volatile uint8_t line_middle;
extern volatile uint32_t exposureTime;


void initModesAndClock(void) {
        ME.MER.R = 0x0000001D;          /* Enable DRUN, RUN0, SAFE, RESET modes */
                                  /* Initialize PLL before turning it on: */
/* Use 1 of the next 2 lines depending on crystal frequency: */
        CGM.FMPLL_CR.R = 0x02400100;    /* 8 MHz xtal: Set PLL0 to 64 MHz */   
/*CGM.FMPLL_CR.R = 0x12400100;*/  /* 40 MHz xtal: Set PLL0 to 64 MHz */   
        ME.RUN[0].R = 0x001F0074;       /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL */
        ME.RUNPC[1].R = 0x00000010;       /* Peri. Cfg. 1 settings: only run in RUN0 mode */
        ME.PCTL[32].R = 0x01;                   /* MPC56xxB ADC 0: select ME.RUNPC[1] */
        ME.PCTL[57].R = 0x01;                   /* MPC56xxB CTUL: select ME.RUNPC[1] */
        ME.PCTL[48].R = 0x01;                   /* MPC56xxB/P/S LINFlex 0: select ME.RUNPC[1] */
        ME.PCTL[68].R = 0x01;                   /* MPC56xxB/S SIUL:  select ME.RUNPC[1] */
        ME.PCTL[72].R = 0x01;                   /* MPC56xxB/S EMIOS 0:  select ME.RUNPC[1] */
        ME.PCTL[73].R = 0x01;                   /* MPC56xxB/S EMIOS 1:  select ME.RUNPC[1] */
        ME.PCTL[92].R = 0x01;           /* PIT, RTI: select ME_RUN_PC[1] */
        ME.PCTL[32].R = 0x01;                   /* ADC: select ME.RUNPC[1] */
                                  /* Mode Transition to enter RUN0 mode: */
        ME.MCTL.R = 0x40005AF0;         /* Enter RUN0 Mode & Key */
        ME.MCTL.R = 0x4000A50F;         /* Enter RUN0 Mode & Inverted Key */  
        while (ME.GS.B.S_MTRANS) {}     /* Wait for mode transition to complete */    
                                  /* Note: could wait here using timer and/or I_TC IRQ */
        while(ME.GS.B.S_CURRENTMODE != 4) {} /* Verify RUN0 is the current mode */
}

void initPeriClkGen(void) {
        CGM.SC_DC[0].R = 0x80;                          /* MPC56xxB/S: Enable peri set 1 sysclk divided by 1 */
        CGM.SC_DC[2].R = 0x80;                  /* MPC56xxB: Enable peri set 3 sysclk divided by 1*/
}

void disableWatchdog(void) {
        SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
        SWT.SR.R = 0x0000d928; 
        SWT.CR.R = 0x8000010A;     /* Clear watchdog enable (WEN) */
}        

void initPIT(void) {
        PIT.PITMCR.R = 0x00000001;                                       /* Enable PIT and configure to stop in debug mode */
        PIT.CH[1].LDVAL.R = (STEERING_CONTROL_INTERVAL-10)*64000-INIT_EXPOSURE_TIME*64;
        																 /* Timeout = STEERING_C_I-10-EXPOSURE_TIME/1000 ms */
        PIT.CH[1].TCTRL.R = 0x000000003;                                 /* Enable PIT1 interrupt & start PIT counting */ 
        PIT.CH[2].LDVAL.R = STEERING_CONTROL_INTERVAL*64000;             /* Timeout= 10*64000 sysclks x 1sec/64M sysclks = 10 ms */
        PIT.CH[2].TCTRL.R = 0x000000003;                                 /* Enable PIT2 interrupt & start PIT counting */
        PIT.CH[3].LDVAL.R = SPEED_CONTROL_INTERVAL*64000;				 /* Timeout= 10*64000 sysclks x 1sec/64M sysclks = 10 ms */
        PIT.CH[3].TCTRL.R = 0x000000003;                                 /* Enable PIT2 interrupt & start PIT counting */ 
        INTC.PSR[61].R = 0x01;                                           /* PIT 2 interrupt vector with priority 1 */
        INTC.PSR[60].R = 0x01;                                           /* PIT 1 interrupt vector with priority 1 */
        INTC.PSR[127].R = 0x01;                                          /* PIT 3 interrupt vector with priority 1 */
}

void initSwIrq4(void) {
        INTC.PSR[4].R = 2;                              /* Software interrupt 4 IRQ priority = 2 */
}

void enableIrq(void) {
        INTC.CPR.B.PRI = 0;          /* Single Core: Lower INTC's current priority */
        asm(" wrteei 1");                  /* Enable external interrupts */
}

void initEMIOS_0(void) {  
        EMIOS_0.MCR.B.GPRE= 63;                         /* Divide 64 MHz sysclk by 63+1 = 64 for 1MHz eMIOS clk*/
        EMIOS_0.MCR.B.GPREN = 1;                        /* Enable eMIOS clock */
        EMIOS_0.MCR.B.GTBE = 1;                         /* Enable global time base */
        EMIOS_0.MCR.B.FRZ = 1;                          /* Enable stopping channels when in debug mode */
}

void initEMIOS_1(void) {  
        EMIOS_1.MCR.B.GPRE= 63;                 /* Divide 64 MHz sysclk by 63+1 = 64 for 1MHz eMIOS clk*/
        EMIOS_1.MCR.B.GPREN = 1;                /* Enable eMIOS clock */
        EMIOS_1.MCR.B.GTBE = 1;                 /* Enable global time base */
        EMIOS_1.MCR.B.FRZ = 1;                  /* Enable stopping channels when in debug mode */
}

void initEMIOS_0ch0(void) {                     /* EMIOS 0 CH 0: Modulus Up Counter */
        EMIOS_0.CH[0].CADR.R = 19999;           /* Period will be 19999+1 = 20000 clocks (20 msec)*/
        EMIOS_0.CH[0].CCR.B.MODE = 0x50;        /* Modulus Counter Buffered (MCB) */
        EMIOS_0.CH[0].CCR.B.BSL = 0x3;          /* Use internal counter */
        EMIOS_0.CH[0].CCR.B.UCPRE=0;            /* Set channel prescaler to divide by 1 */
        EMIOS_0.CH[0].CCR.B.UCPEN = 1;          /* Enable prescaler; uses default divide by 1*/
        EMIOS_0.CH[0].CCR.B.FREN = 1;           /* Freeze channel counting when in debug mode*/
}

void initEMIOS_0ch23(void) {            /* EMIOS 0 CH 23: Modulus Up Counter */
        EMIOS_0.CH[23].CADR.R = 999;            /* Period will be 999+1 = 1000 clocks (1 msec)*/
        EMIOS_0.CH[23].CCR.B.MODE = 0x50;       /* Modulus Counter Buffered (MCB) */
        EMIOS_0.CH[23].CCR.B.BSL = 0x3;         /* Use internal counter */
        EMIOS_0.CH[23].CCR.B.UCPRE=0;           /* Set channel prescaler to divide by 1 */
        EMIOS_0.CH[23].CCR.B.UCPEN = 1;         /* Enable prescaler; uses default divide by 1*/
        EMIOS_0.CH[23].CCR.B.FREN = 1;          /* Freeze channel counting when in debug mode*/
}

void initEMIOS_0ch8(void) {             /* EMIOS 0 CH 23: Modulus Up Counter */
        EMIOS_0.CH[8].CADR.R = 99999;           /* Period will be 99999+1 = 100000 clocks (100 msec)*/
        EMIOS_0.CH[8].CCR.B.MODE = 0x50;        /* Modulus Counter Buffered (MCB) */
        EMIOS_0.CH[8].CCR.B.BSL = 0x3;          /* Use internal counter */
        EMIOS_0.CH[8].CCR.B.UCPRE=0;            /* Set channel prescaler to divide by 1 */
        EMIOS_0.CH[8].CCR.B.UCPEN = 1;          /* Enable prescaler; uses default divide by 1*/
        EMIOS_0.CH[8].CCR.B.FREN = 1;           /* Freeze channel counting when in debug mode*/
}

void initPads()
{
        SIU.PCR[20].R = 0x2000;                 /* MPC56xxB: Initialize PB[4] as ANP0 */
        SIU.PCR[21].R = 0x2000;                 /* MPC56xxB: Initialize PB[5] as ANP1 */
        SIU.PCR[22].R = 0x2000;                 /* MPC56xxB: Initialize PB[6] as ANP2 */
}

/* new line capture signal */
void Pit1ISR(void) {
	uint8_t i=0;
	
	PIT.CH[1].TCTRL.R = 0x000000000;			/* Disable PIT1 interrupt & disable PIT counting */
	if(flag_lineDone==-1)
	{
		TriggerCamera();
		DumpCameraBuffer();
		flag_lineDone = 0;
		PIT.CH[1].LDVAL.R = exposureTime*64;	/* Timeout = exposureTime us */
 	}
	else if(flag_lineDone==0)
	{
		newLine();
		flag_lineDone=1;
		PIT.CH[1].LDVAL.R = (STEERING_CONTROL_INTERVAL)*64000-exposureTime*64;
	}
	else
	{
		PIT.CH[1].LDVAL.R = 500;	/* Timeout = 500 us */
		flag_lineDone = -1;
	}
	
	PIT.CH[1].TCTRL.R = 0x000000003;			/* Enable PIT1 interrupt & start PIT counting */
	PIT.CH[1].TFLG.B.TIF = 1;    				/* MPC56xxP/B/S: CLear PIT 1 flag by writing 1 */
}

void Pit2ISR(void)
{
        SteeringController();
        PIT.CH[2].TFLG.B.TIF = 1;    /* MPC56xxP/B/S: CLear PIT 2 flag by writing 1 */
}

void Pit3ISR()
{
        VelocityController();
        PIT.CH[3].TFLG.B.TIF = 1;    /* MPC56xxP/B/S: CLear PIT 2 flag by writing 1 */
}

/* camera pre-processing */
void SwIrq4ISR(void) {
    
	LineProcessing();
	
	flag_lineDone = -1;
    
    INTC.SSCIR[4].R = 1;            /* Clear channel's flag */
}

void main (void) {      
        volatile uint32_t i = 0;                        /* Dummy idle counter */
        uint8_t success=0;
        uint8_t byteReceived=0;
        uint8_t opcode=0;
        uint8_t payload=0;
        char    msg[32];
        uint8_t msgLength=0;
        clock = 0;
          
        initModesAndClock();  /* MPC56xxP/B/S: Initialize mode entries, set sysclk = 64 MHz*/
        initPeriClkGen();       /* Initialize peripheral clock generation for DSPIs */
        disableWatchdog();    /* Disable watchdog */
        EXCEP_InitExceptionHandlers();          /* Initialize exceptions: only need to load IVPR */
        initADC();
        initPIT();                      /* Initialize PIT1 for 10Hz IRQ, priority 2 */
        initPads();                     /* Initialize software interrupt 4 */
        initEMIOS_0();        /* Initialize eMIOS channels as counter, SAIC, OPWM */
        initEMIOS_1();        /* Initialize eMIOS channels as counter, SAIC, OPWM */
                        
        initEMIOS_0ch0();       /* Initialize eMIOS 0 channel 0 as modulus counter*/
        initEMIOS_0ch23();      /* Initialize eMIOS 0 channel 23 as modulus counter*/
        initEMIOS_0ch8();       /* Initialize eMIOS 0 channel 8 as modulus counter*/
        
        
        //just to make sure the wheels are facing straight
        initDrive();
        Drive();
        
        SIU.PCR[64].R = 0x0100;                         /* Program the drive enable pin of S1 (PE0) as input*/
        while((SIU.PGPDI[2].R & 0x80000000) == 0x80000000); /*Wait until S1 switch is pressed*/
        for(i=0;i<100000;i++);
        while((SIU.PGPDI[2].R & 0x80000000) != 0x80000000); /*Wait until S1 switch is released*/
          
        INTC_InitVector();
        INTC_InstallINTCInterruptHandler(&SwIrq4ISR,4,3);
        INTC_InstallINTCInterruptHandler(&EOC_ISR,62,5);
        INTC_InstallINTCInterruptHandler(&Pit1ISR,60,2);
        INTC_InstallINTCInterruptHandler(&Pit2ISR,61,4);
        INTC_InstallINTCInterruptHandler(&Pit3ISR,127,4);
        
        initSerial();
          
        flag_lineDone = -1;
        initCamera();
        
        initSteeringController();
        
        INTC_InitINTCInterrupts();
        INTC.CPR.B.PRI = 0;          /* Single Core: Lower INTC's current priority */
          
        initDrive();
        
//      setPWMRw(48);
//      setPWMLw(48);
        setDirection(FORWARD);
                
        in = getInBuffer();
        out = getOutBuffer();
                
        MESSAGE("I'm running\n\r");
        fifo_write(&out->fifo,msg,msgLength);
        
        while(isCameraReady()!=STATE_READY);
 
        for(;;)
        {               
                if(!(flag_lineDone==1))
                {
                        Drive();
                        success=0;
                        success = fifo_read(&in->fifo,&byteReceived,1);
                        if(success==1)
                        {
                                if(opcode==0)
                                {
                                        opcode = byteReceived;
                                        MESSAGE("Command Received: ");
                                        fifo_write(&out->fifo,msg,msgLength);
                                }
                                else
                                {
                                        payload = byteReceived;
                                        switch(opcode)
                                        {
                                                case DRIVE:
        //                                              TransmitData("Drive Command\n\r");
                                                        MESSAGE("Drive Command\n\r");
                                                        fifo_write(&out->fifo,msg,msgLength);
                                                        setDirection(payload);
                                                        break;
                                                case SET_SPEED:
        //                                              TransmitData("Set Speed Command\n\r");
                                                        MESSAGE("Set Speed Command\n\r");
                                                        fifo_write(&out->fifo,msg,msgLength);
                                                        setPWMRw(payload);
                                                        setPWMLw(payload);
                                                        break;
                                                case STEERING:
        //                                              TransmitData("Set Steering Command\n\r");
                                                        MESSAGE("Set Steering Command\n\r");
                                                        fifo_write(&out->fifo,msg,msgLength);
                                                        setAngle(payload);
                                                        break;
                                                default:
        //                                              TransmitData("Bad command\n\r");
                                                        MESSAGE("Bad command\n\r");
                                                        fifo_write(&out->fifo,msg,msgLength);
                                                        break;
                                        }
                                        opcode = 0;
                                }
                        }
                }
                
                if(TRANSMISSION_DONE()&&(out->fifo.length>0))
                        Tx();
                
                if(RECEPTION_DONE()&&(in->fifo.length<IN_BUFFER_SIZE))
                        Rx();

        }       
}
