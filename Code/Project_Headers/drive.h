/********************************************/
/* 	Drive State Machine						*/
/* 	                                        */
/*	                                        */
/********************************************/
#include "MPC5604B.h"

#ifndef _DRIVE_H_
#define _DRIVE_H_

/* pins and pwms used */
//#define F_PWM 		EMIOS_0.CH[6]
//#define R_PWM 		EMIOS_0.CH[7]
//#define F_PCR 		SIU.PCR[30]
//#define R_PCR 		SIU.PCR[31]
//#define F_PIN 		SIU.GPDO[30]
//#define R_PIN 		SIU.GPDO[31]
//#define SERVO_PWM	EMIOS_0.CH[4]
//#define SERVO_PCR	SIU.PCR[28]

#define L_PWM 			EMIOS_0.CH[4]
#define R_PWM 			EMIOS_0.CH[7]
#define L_PCR 			SIU.PCR[28]
#define R_PCR 			SIU.PCR[31]
#define L_BRK_PCR		SIU.PCR[29]
#define L_BRK_PIN		SIU.GPDO[29]
#define R_BRK_PCR		SIU.PCR[26]
#define R_BRK_PIN		SIU.GPDO[26]
#define R_ENC	 		EMIOS_1.CH[8]	//PH6
#define L_ENC	 		EMIOS_0.CH[24]	//PD12
#define R_ENC_PCR 		SIU.PCR[118]
#define L_ENC_PCR 		SIU.PCR[60]
#define SERVO_PWM		EMIOS_0.CH[3]
#define SERVO_PCR		SIU.PCR[27]


/* direction values */
#define IDLE	0x00
#define FORWARD 0x01
#define REVERSE	0x02
#define BREAK	0x03


/* steering directions */
#define MIDDLE 		2050
#define MAX_RIGHT	2400
#define MAX_LEFT	1700


void initDrive();	/* initializes the state machine */
void Drive();		/* executes the state machine */
void setDirection(uint8_t d);
void BreakLeftWheel();
void BreakRightWheel();
void RunLeftWheel();
void RunRightWheel();
uint8_t getPWMRight();
uint8_t getPWMLeft();
void setPWMRw(uint8_t s);
void setPWMLw(uint8_t s);
void setAngle(uint8_t a);
int8_t getAngle();
uint16_t getLeftEncoder();
uint16_t getRightEncoder();

#endif
