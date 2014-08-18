/********************************************/
/* 	Drive State Machine						*/
/* 	                                        */
/*	                                        */
/********************************************/

#include "drive.h"

/* state variables */
char speed_lw,speed_rw;
int8_t angle;
char direction;
volatile uint16_t prev_encoder_r,prev_encoder_l;


void initDrive()
{
	speed_rw = 0;
	speed_lw = 0;
	angle = 0;
	direction = IDLE;
	prev_encoder_r=0;
	prev_encoder_l=0;
	
//	/* sets the forward pwm */
//	F_PWM.CADR.R = 0;     		/* Leading edge when channel counter bus=0*/
//	F_PWM.CBDR.R = 0;     		/* Trailing edge when channel counter bus=0*/
//	F_PWM.CCR.B.BSL = 0x0;  	/* Use counter bus A (default) */
//	F_PWM.CCR.B.EDPOL = 1;  	/* Polarity-leading edge sets output */
//	F_PWM.CCR.B.MODE = 0x60; 	/* Mode is OPWM Buffered */
//	F_PCR.R = 0x0600;		    /* MPC56xxS:*/
//	
//	/* sets the reverse pwm */
//	R_PWM.CADR.R = 0;     		/* Leading edge when channel counter bus=0*/
//	R_PWM.CBDR.R = 0;     		/* Trailing edge when channel counter bus=0*/
//	R_PWM.CCR.B.BSL = 0x0;  	/* Use counter bus A (default) */
//	R_PWM.CCR.B.EDPOL = 1;  	/* Polarity-leading edge sets output */
//	R_PWM.CCR.B.MODE = 0x60; 	/* Mode is OPWM Buffered */
//	R_PCR.R = 0x0600;		    /* MPC56xxS:*/
	
	/* sets the left wheel pwm */
	L_PWM.CADR.R = 0;     		/* Leading edge when channel counter bus=0*/
	L_PWM.CBDR.R = 0;     		/* Trailing edge when channel counter bus=0*/
	L_PWM.CCR.B.BSL = 0x0;  	/* Use counter bus A (default) */
	L_PWM.CCR.B.EDPOL = 1;  	/* Polarity-leading edge sets output */
	L_PWM.CCR.B.MODE = 0x60; 	/* Mode is OPWM Buffered */
	L_PCR.R = 0x0600;		    /* MPC56xxS:*/
	
	/* sets the right wheel pwm */
	R_PWM.CADR.R = 0;     		/* Leading edge when channel counter bus=0*/
	R_PWM.CBDR.R = 0;     		/* Trailing edge when channel counter bus=0*/
	R_PWM.CCR.B.BSL = 0x0;  	/* Use counter bus A (default) */
	R_PWM.CCR.B.EDPOL = 1;  	/* Polarity-leading edge sets output */
	R_PWM.CCR.B.MODE = 0x60; 	/* Mode is OPWM Buffered */
	R_PCR.R = 0x0600;		    /* MPC56xxS:*/

	
	// sets the left wheel encoder 
	L_ENC.CADR.R = 0xFFFF;      /* Period will be 99999+1 = 100000 clocks (100 msec)*/
	L_ENC.CCR.B.MODE = 0x51; 	/* Modulus Counter Buffered (MCB) */
	L_ENC.CCR.B.BSL = 0x3;   	/* Use internal counter */
	L_ENC.CCR.B.UCPRE=0;     	/* Set channel prescaler to divide by 1 */
	L_ENC.CCR.B.UCPEN = 1;   	/* Enable prescaler; uses default divide by 1*/
	L_ENC.CCR.B.FREN = 1;   	/* Freeze channel counting when in debug mode*/
	L_ENC_PCR.B.PA = 2;
	L_ENC_PCR.B.IBE = 1;
	
	//sets the right wheel encoder 
	R_ENC.CADR.R = 0xFFFF;      /* Period will be 99999+1 = 100000 clocks (100 msec)*/
	R_ENC.CCR.B.MODE = 0x51; 	/* Modulus Counter Buffered (MCB) */
	R_ENC.CCR.B.BSL = 0x3;   	/* Use internal counter */
	R_ENC.CCR.B.UCPRE=0;     	/* Set channel prescaler to divide by 1 */
	R_ENC.CCR.B.UCPEN = 1;   	/* Enable prescaler; uses default divide by 1*/
	R_ENC.CCR.B.FREN = 1;   	/* Freeze channel counting when in debug mode*/
	R_ENC_PCR.R = 0x0500;		//     MPC56xxS:
	
	/* servo pwm */
	SERVO_PWM.CADR.R = 0;     		/* Leading edge when channel counter bus=0*/
	SERVO_PWM.CBDR.R = MIDDLE+angle/100*(MAX_RIGHT-MIDDLE);      	/* Trailing edge when channel counter bus=1400 Middle, 1650 Right Max, 1150 Left Max*/
	SERVO_PWM.CCR.B.BSL = 0x01;  	/* Use counter bus B */
	SERVO_PWM.CCR.B.EDPOL = 1;  	/* Polarity-leading edge sets output */
	SERVO_PWM.CCR.B.MODE = 0x60; 	/* Mode is OPWM Buffered */
	SERVO_PCR.R = 0x0600;           /* MPC56xxS: Assign EMIOS_0 ch 4 to pad */
}

void setDirection(uint8_t d)
{
	direction = d;
}

uint8_t getPWMRight()
{
	return speed_rw;
}

uint8_t getPWMLeft()
{
	return speed_lw;
}

void setPWMRw(uint8_t s)
{
	speed_rw = s;
}

void setPWMLw(uint8_t s)
{
	speed_lw = s;
}

int8_t getAngle()
{
	return angle;
}

void setAngle(uint8_t a)
{
	angle = (int8_t)a;
}

uint16_t getLeftEncoder()
{
	int16_t period;
	uint16_t current = L_ENC.CCNTR.B.CCNTR;
	period = current - prev_encoder_l;
	prev_encoder_l = current;
	
	if(period<0)
		return (uint16_t)(period + 65535);
	else	
		return (uint16_t)period;
}

uint16_t getRightEncoder()
{
	int16_t period;
	uint16_t current = R_ENC.CCNTR.B.CCNTR;
	period = current - prev_encoder_r;
	prev_encoder_r = current;
	
	if(period<0)
		return (uint16_t)(period + 65535);
	else	
		return (uint16_t)period;
}

void Drive()
{
	switch(direction)
	{
		case IDLE:
//			F_PCR.R = 0x0200;
//			F_PIN.R = 0;
//			R_PCR.R = 0x0200;
//			R_PIN.R = 0;
			L_PCR.R = 0x0600;
			L_PWM.CBDR.R = 0;
			R_PCR.R = 0x0600;
			R_PWM.CBDR.R = 0;
		break;
		
		case FORWARD:
			L_PCR.R = 0x0600;
			L_PWM.CBDR.R = speed_lw*10;
			R_PCR.R = 0x0600;
			R_PWM.CBDR.R = speed_rw*10;
		break;
		
		case REVERSE:
//			F_PCR.R = 0x0200;
//			F_PIN.R = 0;
//			R_PCR.R = 0x0600;
//			R_PWM.CBDR.R = speed*10;
		break;
		
		case BREAK:
			speed_lw = 0;
			L_BRK_PCR.R = 0x0200;
			L_BRK_PIN.R = 1;
			speed_rw = 0;
			R_BRK_PCR.R = 0x0200;
			R_BRK_PIN.R = 1;
		break;
		
	}
	
	/* servo update */
	if(angle>=0)
		SERVO_PWM.CBDR.R = (uint32_t)(MIDDLE+angle/100.0*(MAX_RIGHT-MIDDLE));
	else
		SERVO_PWM.CBDR.R = (uint32_t)(MIDDLE+angle/100.0*(MIDDLE-MAX_LEFT));
	
}

void BreakLeftWheel()
{
	speed_lw = 0;
	L_BRK_PCR.R = 0x0200;
	L_BRK_PIN.R = 1;
}
void BreakRightWheel()
{
	speed_rw = 0;
	R_BRK_PCR.R = 0x0200;
	R_BRK_PIN.R = 1;
}

void RunLeftWheel()
{
	L_BRK_PCR.R = 0x0200;
	L_BRK_PIN.R = 0;
}
void RunRightWheel()
{
	R_BRK_PCR.R = 0x0200;
	R_BRK_PIN.R = 0;
}
