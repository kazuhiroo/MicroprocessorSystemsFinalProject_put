/**
 * @file Filters.h
 * @brief This file contains global variables.
 *
 * This file provides global variables and its start-up parameters
 * for signal processing within the control loop, such as communication and steering.
 *
 *
 * @author  a.katowski, https://github.com/kazuhiroo
 * @author  o.jozwik, https://github.com/oskarjozwik
 * @author  i.kolas, https://github.com/ililich
 * @date    2026-02-01
 */

#ifndef INC_GLOBAL_VARIABLES_H_
#define INC_GLOBAL_VARIABLES_H_

#define FB_TIMER_FREQ       1000000.0f
#define ENC_PULSES_PER_REV  20 // signals per single rotation from the encoding disc
#define MAX_SPEED 			300.0f // max speed for 10V from the VC
#define MIN_SPEED 			50.0f // max speed for 10V from the VC
#define ENC_CONST 			2
#define START_SPEED 		100.0f


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Control.h"

extern _Bool ic_start_flag;
extern uint32_t ic_prev;
extern uint32_t u_global;
extern float life_timer;

extern char UART_Message[];
extern uint8_t UART_MessageLen;
extern _Bool UART_ReceiveFlag;
extern _Bool UART_TransmitFlag;
extern uint8_t UART_TransmitCnt;


extern uint8_t ENC_Cnt;
extern uint8_t ENC_Cnt_prev;
extern _Bool USER_Btn_flag;

extern PID Pid1;



#endif /* INC_GLOBAL_VARIABLES_H_ */
