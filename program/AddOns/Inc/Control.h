/**
 * @file Control.h
 * @brief Public interface of a PID controller for a 3–6 V DC motor.
 *
 * This file defines the PID controller structure, configuration
 * parameters, and functions required to control a DC motor
 * in a closed-loop system using a PID algorithm.
 *
 * @author  a.katowski, https://github.com/kazuhiroo
 * @date    2026-01-17
 */

#ifndef PID_H_
#define PID_H_

// object parameters G(s) = K/(sT+1)
#define T 0.728823f
#define K 1.5277f

// PID controller parameters

#define KP 2.2f
#define KI 3.5f
#define KD 0.085f
#define KFF 0.0f

#define CLOSED_LOOP 1

// saturation values
#define U_SAT_UP            1000.0f
#define U_SAT_DOWN          0.0f

#define PWM_MIN       		0
#define PWM_MAX 			1000
#define SAMPLING_PERIOD 	0.01f

#define DEADZONE_PWM		500

/**
 * @brief PID controller structure.
 *
 * Stores controller gains, internal states, and input/output values.
 */
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

/**
 * @brief Updates the output of the PID controller.
 *
 * Computes a new control signal (PWM duty cycle) based on
 * the current error and controller parameters.
 *
 * @param pid Pointer to the PID controller structure
 */
void PID_update(PID *pid);

/**
 * @brief Resets the internal state of the PID controller.
 *
 * Returns PID to starting state, setting values of every signal to 0
 * 
 *
 * @param pid Pointer to the PID controller structure
 */
void PID_reset(PID *pid);

/**
 * @brief Compensates the deadzone caused by motor friction.
 *
 * Returns fixed signal if value of signal 
 * u falls below a threshold for starting 
 * motor rotation.
 *
 * @param u Raw control signal
 * @return Control signal after deadzone compensation
 */
float Deadzone_compensation(float u);


#endif /* PID_H_ */
