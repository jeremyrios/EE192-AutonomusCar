/********************************************/
/* 	Utilities								*/
/* 	                                        */
/*	                                        */
/********************************************/

#ifndef _UTILITIES_H_
#define _UTILITIES_H_

typedef struct {
     char * buf;
     int head;
     int tail;
     int size;
     int length;
} fifo_t;

void fifo_init(fifo_t * f, char * buf, int size);
int fifo_read(fifo_t * f, void * buf, int nbytes);
int fifo_write(fifo_t * f, const void * buf, int nbytes);

#endif