/*
  * @file    : PID.h
  * @author  : a.katowski, https://github.com/kazuhiroo
  * @date    : Jan 17, 2026
  * @brief   : PID controller algorithm implementation for a sa 3-6V DC Motor.
  *
*/

#ifndef PID_H_
#define PID_H_

// PID controller parameters
#define KP 1.0f
#define KI 0.0f
#define KD 0.0f

// saturation values
#define U_SAT_UP            1000.0f
#define U_SAT_DOWN          0.0f

#define PWM_MIN_START       800.0f
#define SAMPLING_PERIOD 	0.01f

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
void PID_Reset(PID *pid);

#endif /* PID_H_ */
