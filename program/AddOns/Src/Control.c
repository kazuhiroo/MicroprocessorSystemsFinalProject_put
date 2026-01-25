/*
  * @file    : Control.c
  * @author  : a.katowski, https://github.com/kazuhiroo
  * @date    : Jan 17, 2026
  * @brief   : PID controller algorithm implementation for a 3-6V DC Motor.
  *
*/

#include <Control.h>

/*
 * @brief   : function updating the output of the PID controller ( steering signal: PWM duty)
 *
 * */
void PID_update(PID *pid){
    float e;

    if(CLOSED_LOOP){
    	e = pid->y_ref - pid->y; // calculate current error
    }
    else{
    	e = pid->y_ref;
    }

    // proportional
    pid->up = pid->Kp * e;

    // integrator
    pid->ui += pid->Ki * e * SAMPLING_PERIOD;

    //antiwind up (clamping)
    if(e > 0.0f){
    	if(pid->ui > 1000.0f) {
    	        pid->ui = 1000.0f;
    	    }
    	else if(pid->ui < 0.0f) {
    	        pid->ui = 0.0f;
    	    }
    }


    // derivative
    pid->ud = pid->Kd * (e - pid->e) / SAMPLING_PERIOD;

    // total u
    float u = pid->up + pid->ui + pid->ud;

    pid->u = Deadzone_compensation(u);

    // remember current error
    pid->e = e;
}

/*
 * @brief   : function for absolute controller reset
 *
 * */
void PID_reset(PID *pid){
	pid->u = 0;
	pid->up = 0;
	pid->ui = 0;
	pid->ud = 0;
	pid->y = 0;
	pid->e = 0;
}

/*
 * @brief   : function for compensation the deadzone due to the friction
 *
 * */
float Deadzone_compensation(float u){
    if (u < DEADZONE_PWM) {
        return DEADZONE_PWM; // minimalne napięcie, aby ruszył silnik
    }
    return u; // jeśli sygnał wystarczająco duży, zostawiamy
}




