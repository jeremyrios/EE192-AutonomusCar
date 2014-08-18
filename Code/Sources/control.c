#include "control.h"

float error, output, previous_error=0; 
float D_error = 0, I_error = 0; 
float cR=0, cL=0;
volatile uint8_t        line_middle;

//for speed control
float speed_output = 0;
/* right wheel parameters */
int16_t PWM_out_r = 0;
float prev_speed_error_r = 0;
float v_set_r = 6.0;
float v_des_r = 0;
float speed_error_r=0;
float D_speed_error_r = 0, I_speed_error_r = 0;

/* left wheel parameters */
int16_t PWM_out_l = 0;
float prev_speed_error_l = 0;
float v_set_l = 6.0;
float v_des_l =0;
float speed_error_l=0;
float D_speed_error_l = 0, I_speed_error_l = 0;

void initSteeringController()
{
        error = 0;
        output = 0; 
        previous_error = 0; 
        D_error = 0;
        I_error = 0;    
        
        line_middle = CENTER_LINE;
}

void SteeringController(void) 
{
  
        float angle=0, xcm =0, out_freq = 0;
        float v_med;
        int8_t percentage = 0;
        uint8_t servo_pwm = 0;

        v_med = (v_des_r+v_des_l)/2;
        //Calculate Proportional Error 
        error =  line_middle - CENTER_LINE;
        
        //Calculate Differential Error
        D_error = (float)((error - previous_error)/_dt_);

        //Calculate Integral Error with windup and the output
        if((output== MAX_R_ANGLE && error>0) || (output==MAX_L_ANGLE && error<0)){
                output = (float)((P_GAIN * error)  - (D_GAIN * D_error));
                I_error = 0;
        }
        else{
                I_error += (error)*_dt_;
                output = (float)((P_GAIN * error) + (I_GAIN*I_error) - (D_GAIN * D_error));
        }
        
        
        //Update Previous error
        previous_error = error;
        
        xcm = (float)(output*TRACK_LENGHT)/128;
        
        angle = (float)atan2(xcm,L);
        
        if(angle>0)
        {
                if(angle>MAX_R_ANGLE){
                        angle = MAX_R_ANGLE;
                }
                percentage = (int8_t)((angle*100)/MAX_R_ANGLE);
        }
        else
        if(angle<0)
        {
                if(angle<MAX_L_ANGLE){
                        angle = MAX_L_ANGLE;
                }
                percentage = (int8_t)((-angle*100)/MAX_L_ANGLE);
        }
        else
        if(angle==0)
        {
                out_freq = 0;
                percentage = 0; 
        }
        
        //update servo
        servo_pwm = (uint8_t)(percentage);
        setAngle(servo_pwm);    
                

}

void VelocityController()
{
        float current_speed;
        uint16_t transitions_l,transitions_r;
        transitions_l = getLeftEncoder();
        transitions_r = getRightEncoder();
        /* right wheel */
        //Calculate Proportional Error 

        v_des_r = v_set_r - KV*fabsf(getAngle())-KD*fabsf(D_error);
        if(v_des_r<1.5)
        	v_des_r = 1.5;
        current_speed = (float)(2*3.14/8*(transitions_r)/_dt_motor*W_RADIUS);
        speed_error_r = v_des_r - current_speed;
        //Calculate Differential Error
        D_speed_error_r = (float)((speed_error_r - prev_speed_error_r)/_dt_motor);
        //Calculate Integral Error and output, considering Anti-windup
        if((PWM_out_r==100 && error>0 )||(PWM_out_r==0 && error<0)){
                speed_output = (float)((P_GAIN_DC * speed_error_r) - (D_GAIN_DC * D_speed_error_r));
                I_speed_error_r = 0;
        }
        else
        {
                I_speed_error_r += speed_error_r*_dt_motor;
                speed_output = (float)((P_GAIN_DC * speed_error_r) + (I_GAIN_DC * I_speed_error_r) - (D_GAIN_DC * D_speed_error_r));
        }

        //Update Previous error
        prev_speed_error_r = speed_error_r;

        //update DC motor speed
        PWM_out_r = (speed_output*100);//getPWMRight() + (int16_t)(speed_output*100);

        if(PWM_out_r>100)
        {
        	PWM_out_r = 100;
        	RunRightWheel();
        }
        else if(PWM_out_r<0)
        {
        	BreakRightWheel();
        	PWM_out_r = 0;
        }
        else
        {
        	RunRightWheel();
        }

        setPWMRw((uint8_t)PWM_out_r);
        
        /* left wheel */
        //Calculate Proportional Error 

        v_des_l = v_set_l - KV*fabsf(getAngle())-KD*fabsf(D_error);

        if(v_des_l<1.5)
        	v_des_l = 1.5;
        current_speed = (float)(2*3.14*(transitions_l/8.0)/_dt_motor*W_RADIUS);
        speed_error_l = v_des_l - current_speed;
        //Calculate Differential Error
        D_speed_error_l = (float)((speed_error_l - prev_speed_error_l)/_dt_motor);
        //Calculate Integral Error and output, considering Anti-windup
        if((PWM_out_l==100 && error>0 )||(PWM_out_l==0 && error<0)){
                speed_output = (float)((P_GAIN_DC * speed_error_l) - (D_GAIN_DC * D_speed_error_l));
                I_speed_error_l = 0;
        }
        else
        {
                I_speed_error_l += speed_error_l*_dt_motor;
                speed_output = (float)((P_GAIN_DC * speed_error_l) + (I_GAIN_DC * I_speed_error_l) - (D_GAIN_DC * D_speed_error_l));
        }

        //Update Previous error
        prev_speed_error_l = speed_error_l;

        //update DC motor speed
        PWM_out_l = (speed_output*100);//getPWMLeft() + (int16_t)(speed_output*100);

        if(PWM_out_l>100)
        {
        	PWM_out_l = 100;
        	RunLeftWheel();
        }
        else if(PWM_out_l<0)
        {
        	BreakLeftWheel();
        	PWM_out_l = 0;
        }
        else
        {
        	RunLeftWheel();
        }

        setPWMLw((uint8_t)PWM_out_l);
}
