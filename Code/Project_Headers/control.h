/*
 * control.h
 *
 *  Created on: Mar 15, 2013
 *      Author: Thiago Ton
 */

#ifndef CONTROL_H_
#define CONTROL_H_
#include "drive.h"
#include "camera.h"
#include <cmath>


#define CENTER_LINE 64
#define SPEED_CONTROL_INTERVAL         50 //100 //ms
#define STEERING_CONTROL_INTERVAL       30      //ms
#define P_GAIN 1.2//0.98
#define I_GAIN 0.0//0.09
#define D_GAIN 0.05//0.048//
//#define P_GAIN 0.9  //working =)
//#define I_GAIN 0.1
//#define D_GAIN 0.05 //@ 2 meter/sec
//#define KV 0.02
#define _dt_            (STEERING_CONTROL_INTERVAL/1000.0) //s 
#define _dt_motor       (SPEED_CONTROL_INTERVAL/1000.0) //s
#define L   10  //h = 20, d = 22, l = sqrt(22^2 - h^2), em cm
#define TRACK_LENGHT 21.5 //quantos cm valem os 128 bits
#define MAX_R_ANGLE 1.047
#define MAX_L_ANGLE -1.047

//for the speed control
#define W_RADIUS 0.025 //in meters
#define W_DIVISION 16
#define P_GAIN_DC 1.5
#define I_GAIN_DC 0.01
#define D_GAIN_DC 0.0
#define KV 0.5//0.01//0.1 //how much car need to slow down on curves
#define KD 0.02
void initSteeringController();

void SteeringController(void) ;

void VelocityController();

#endif /* CONTROL_H_ */
