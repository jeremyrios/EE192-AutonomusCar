/********************************************/
/* 	Protocol Descriptor						*/
/* 	                                        */
/*	Describes the serial protocol used      */
/********************************************/

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_


/* packet fields */
#define OPCODE 	0
#define PAYLOAD 1

/* opcodes */
#define DRIVE 		0x11
#define SET_SPEED	0x12
#define STEERING	0x13
#define SINE_PATH	0x14


/* DRIVE command */
#define DIRECTION 	(PAYLOAD+1)


/* SET_SPEED command */
#define SPEED		(PAYLOAD+1)


/* STEERING command */
#define ANGLE		(PAYLOAD+1)


#endif
