/*
  * @file     Global_Variables.c
 * @author  a.katowski, https://github.com/kazuhiroo
 * @author  o.jozwik, https://github.com/oskarjozwik
 * @author  i.kolas, https://github.com/ililich
  * @date     Feb 1, 2026
  * @brief    Implemented global variables for handling the control system
  *
*/
#include "Global_Variables.h"
/*
 * @brief   : variables handling the speed calculating.
 *
 * */
_Bool ic_start_flag = 1;
uint32_t ic_prev = 0;
uint32_t u_global = 0;
float life_timer = 0;


/*
 * @brief   : variables handling UART.
 *
 * */
char UART_Message[] = "000";
uint8_t UART_MessageLen = 3;

_Bool UART_ReceiveFlag = 0;
_Bool UART_TransmitFlag = 0;
uint8_t UART_TransmitCnt = 0;

/*
 * @brief   : variables handling entry value encoder and user button.
 *
 * */
uint8_t ENC_Cnt = START_SPEED;
uint8_t ENC_Cnt_prev = START_SPEED;
_Bool USER_Btn_flag = 0;

/*
 * @brief   : PID global struct.
 *
 * */
PID Pid1 = {
		.Kp = KP,
		.Ki = KI,
		.Kd = KD,

		.e  = 0.0f,
		.u  = 0.0f,
		.up = 0.0f,
		.ui = 0.0f,
		.ud = 0.0f,

		.y = 0.0f,
		.y_ref = START_SPEED
};
