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
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
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

char UART_Message[] = "000";
uint8_t UART_MessageLen = 3;
_Bool UART_ReceiveFlag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    if (GPIO_Pin == USER_Btn_Pin){
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3){
    	UART_ReceiveFlag = 1;
    	HAL_UART_Receive_IT(&huart3, (uint8_t*)UART_Message, 3);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart3, (uint8_t*)UART_Message, 3);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim6);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if (USER_Btn_flag){
		  USER_Btn_flag = 0;
		  speed_ref_new += 10.0f;
		  if (speed_ref_new > MAX_SPEED) speed_ref_new = 0.0f;
	  }
	  if(UART_ReceiveFlag){
		UART_ReceiveFlag = 0;
		UART_Message[UART_MessageLen] = '\0';
		int rx = atoi(UART_Message);

		if(rx >= MAX_SPEED) speed_ref_new = MAX_SPEED;
		else speed_ref_new = rx;
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
