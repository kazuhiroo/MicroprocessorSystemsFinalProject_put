/*
  * @file    : Control.h
  * @author  : a.katowski, https://github.com/kazuhiroo
  * @date    : Jan 17, 2026
  * @brief   : PID controller algorithm implementation for a sa 3-6V DC Motor.
  *
*/

#ifndef PID_H_
#define PID_H_

// object parameters G(s) = K/(sT+1)
#define T 0.728823f
#define K 1.5277f

// PID controller parameters
#define KP 2.0f
#define KI 2.0f
#define KD 1.0f
#define KFF 0.0f

#define CLOSED_LOOP 1

// saturation values
#define U_SAT_UP            1000.0f
#define U_SAT_DOWN          0.0f

#define PWM_MIN       		500
#define PWM_MAX 			1000
#define SAMPLING_PERIOD 	0.01f

#define DEADZONE_PWM		600


typedef struct{
    float Kp;
    float Ki;
    float Kd;

    float e;
    float u;
    float up;
    float ui;
    float ud;

    float y;
    float y_ref;
} PID;

/*
 * @brief   : function updating the output of the PID controller ( steering signal: PWM duty)
 *
 * */
void PID_update(PID *pid);

/*
 * @brief   : function for absolute controller reset
 *
 * */
void PID_reset(PID *pid);
/*
 * @brief   : function for compensation the deadzone due to the friction
 *
 * */
static float Deadzone_compensation(float u);


#endif /* PID_H_ */
