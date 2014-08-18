/********************************************/
/*      Camera                                                                  */
/*                                              */
/*                                              */
/********************************************/
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "MPC5604B.h"
#include "serial.h"
#include "control.h"
#include "math.h"

#define DELAY 10
#define UPPER_EXPOSURE_TIME	19000//us
#define LOWER_EXPOSURE_TIME	50 //us
#define INIT_EXPOSURE_TIME 	19000//us
#define RANGE	7
#define EXPOSURE_STEP		30
#define SEARCH_WINDOW		20
#define MINIMUM_CONTRAST	0.20
#define THRESHOLD			2500

/* camera states */
#define STATE_INITIALIZING	0
#define STATE_READY			1

//#define THRESHOLD_DETECTION 1
//#define WEIGHT_DETECTION 2
//#define THREE_POINTS_DETECTION 3
//#define THREE_POINTS_THRESHOLD_DETECTION 4
//#define THREE_POINTS_WEIGHTED_DETECTION 5
//#define JEREMY_DETECTION
//#define MINIMUM_DETECTION	7
#define CONSECUTIVE_DETECTION 8


#define LINE_WHITE    0
//#define LINE_BLACK 1

#define SI                      120
#define CLK                     116


void initADC(void);
void initCamera();
void EOC_ISR(void);
void newLine();
void TriggerCamera();
void LineProcessing();
void DumpCameraBuffer();
uint8_t isCameraReady();
void FindBestContrast();
float CalculateContrast(uint16_t* min_out,uint16_t* max_out);
void SendRawData();
void SendLine();
void CorrectExposure();


#endif
