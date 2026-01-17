/*
  * @file    : PID.c
  * @author  : a.katowski, https://github.com/kazuhiroo
  * @date    : Jan 17, 2026
  * @brief   : PID controller algorithm implementation for a 3-6V DC Motor.
  *
*/

#include "PID.h"

/*
 * @brief   : function updating the output of the PID controller ( steering signal: PWM duty)
 *
 * */
void PID_update(PID *pid){
    static _Bool start_up = 1;
    float e;

    if(start_up){ // set min value for the DC motor to accelerate
        pid->u = PWM_MIN_START;
        if(pid->y > 50.0){
        	pid->ui = PWM_MIN_START; // signal transfer
        	pid->e = 0.0f; // reset error

            start_up = 0;
        }
        return;
    }

    e = pid->y_ref - pid->y; // calculate current error

    // proportional
    pid->up = pid->Kp * e;

    // integrator with anti-windup
    if(pid->u >= U_SAT_UP || pid->u <= U_SAT_DOWN) pid->ui += 0;
    else pid->ui += pid->Ki * e * SAMPLING_PERIOD;

    // derivative
    pid->ud = pid->Kd * (e - pid->e) / SAMPLING_PERIOD;

    pid->u = pid->up + pid->ui + pid->ud;

    // saturation
    if (pid->u > U_SAT_UP)   pid->u = U_SAT_UP;
    if (pid->u < U_SAT_DOWN) pid->u = U_SAT_DOWN;

    // remember current error
    pid->e = e;
}
