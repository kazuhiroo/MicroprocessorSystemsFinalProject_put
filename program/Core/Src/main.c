/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : main program body
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "eth.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
typedef struct{
    float Kp;
    float Ki;
    float Kd;

    float e;
    float u;
    float up;
    float ui;
    float ud;
} PID;

/* Private define ------------------------------------------------------------*/
#define PWM_MIN_START       760.0f
#define FB_TIMER_FREQ       1000000.0f
#define SAMPLING_PERIOD     0.01f
#define ENC_PULSES_PER_REV  20
#define U_SAT_UP            1000.0f
#define U_SAT_DOWN          0.0f
#define MAX_SPEED 			150.0f


#define KP 2.0f
#define KI 3.0f
#define KD 0.0f

/* Private variables ---------------------------------------------------------*/
void PID_update(PID *pid);

_Bool USER_Btn_flag = 0;

_Bool ic_start_flag = 1;
volatile uint32_t ic_prev = 0;
volatile float speed = 0.0f;
volatile float speed_filt = 0.0f;
volatile float speed_ref = 70.0f;
volatile float speed_ref_new = 70.0f;

volatile float life_timer = 0;
PID Pid1 = {KP, KI, KD, 0, 0, 0, 0, 0};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN 0 */

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        uint32_t now = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint32_t arr = __HAL_TIM_GET_AUTORELOAD(htim);
        uint32_t diff;

        if (ic_start_flag)
        {
            ic_start_flag = 0;
            ic_prev = now;
            return;
        }

        if (now >= ic_prev)
            diff = now - ic_prev;
        else
            diff = (arr - ic_prev) + now + 1;

        ic_prev = now;

        if (diff > 0)
        {
            speed = 60.0f * FB_TIMER_FREQ / (ENC_PULSES_PER_REV * diff);
            speed_filt = 0.8f * speed_filt + 0.2f * speed;
        }

        life_timer = 0.0f;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim6)
    {
        PID_update(&Pid1);

        life_timer += SAMPLING_PERIOD;
        if(life_timer >= 10*SAMPLING_PERIOD){
        	speed = 0;
        	speed_filt = 0;
        	life_timer = 0.0f;
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == USER_Btn_Pin)
    {
        USER_Btn_flag = 1;
    }
}

void PID_update(PID *pid)
{
    static _Bool start_up = 1;
    float e;

    if(speed_ref_new != speed_ref){
    	speed_ref = speed_ref_new;
    	pid->ui = 0.0;
    	e = 0.0f;
    }

    if (start_up){
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, PWM_MIN_START);

        if (speed_filt > 5.0f)     // tylko wykrycie ruchu
        {
            start_up = 0;
            pid->ui = PWM_MIN_START;   // bumpless transfer
            pid->e  = 0.0f;
        }
        return;
    }

    e = speed_ref - speed_filt;

    pid->up = pid->Kp * e;

    if(pid->u >= U_SAT_UP || pid->u <= U_SAT_DOWN) pid->ui += 0;
    else pid->ui += pid->Ki * e * SAMPLING_PERIOD;

    pid->ud = pid->Kd * (e - pid->e) / SAMPLING_PERIOD;

    pid->u = pid->up + pid->ui + pid->ud;

    if (pid->u > U_SAT_UP)   pid->u = U_SAT_UP;
    if (pid->u < U_SAT_DOWN) pid->u = U_SAT_DOWN;

    pid->e = e;

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, (uint32_t)pid->u);
}

/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_ETH_Init();
    MX_I2C1_Init();
    MX_USART3_UART_Init();
    MX_USB_OTG_FS_PCD_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();
    MX_TIM6_Init();

    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
    HAL_TIM_Base_Start_IT(&htim6);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);

    while (1)
    {
        if (USER_Btn_flag){
            USER_Btn_flag = 0;
            speed_ref_new += 10.0f;
            if (speed_ref_new > MAX_SPEED) speed_ref_new = 0.0f;
        }
    }
}

/* ================= CLOCK ================= */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 72;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 3;

    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* ================= ERROR ================= */

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}
