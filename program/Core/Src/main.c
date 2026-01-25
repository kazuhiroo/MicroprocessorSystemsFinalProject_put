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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>

// controller
#include "Control.h"
#include "Filters.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FB_TIMER_FREQ       1000000.0f
#define ENC_PULSES_PER_REV  20 // signals per single rotation from the encoding disc
#define MAX_SPEED 			150.0f // max speed for 5V from the VC
#define ENC_CONST 			2
#define START_SPEED 		100

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
_Bool ic_start_flag = 1;
uint32_t ic_prev = 0;
uint32_t u_global = 0;
float life_timer = 0;

char UART_Message[] = "000";
uint8_t UART_MessageLen = 3;

_Bool UART_ReceiveFlag = 0;
_Bool UART_TransmitFlag = 0;
uint8_t UART_TransmitCnt = 0;
_Bool USER_Btn_flag = 0;

uint8_t ENC_Cnt = 0;
uint8_t ENC_Cnt_prev = 0;


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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
    if (htim == &htim4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
        uint32_t now = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint32_t arr = __HAL_TIM_GET_AUTORELOAD(htim);
        uint32_t diff;

        if (ic_start_flag){ // first input
            ic_start_flag = 0;
            ic_prev = now;
            return;
        }

        if (now >= ic_prev)
            diff = now - ic_prev;
        else
            diff = (arr - ic_prev) + now + 1;
        ic_prev = now;

        if (diff > 0){
        	float speed = 60.0f * FB_TIMER_FREQ / (ENC_PULSES_PER_REV * diff);
        	if(speed >1.5*MAX_SPEED) return;
        	speed = 0.8f * Pid1.y + 0.2f * speed; // small LPF
        	Pid1.y = AvgFilter(speed); // average filter
        }

        life_timer = 0.0f; // motor on - reset life timer
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if (htim == &htim6){
        float u_ff = KFF * Pid1.y_ref;
        PID_update(&Pid1);

        float u_calc = u_ff + Pid1.u;

        if(u_calc > (float)PWM_MAX) u_calc = (float)PWM_MAX;
        if(u_calc < (float)PWM_MIN) u_calc = (float)PWM_MIN;

        u_global = (uint32_t)u_calc;

        life_timer += SAMPLING_PERIOD;
//        if(life_timer >= 1000 * SAMPLING_PERIOD){
//            Pid1.y = 0;
//            u_global = 0;
//            life_timer = 0.0f;
//
//            PID_reset(&Pid1);
//        }

        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, u_global);

        UART_TransmitCnt += 1;
        if(UART_TransmitCnt == 100){
            UART_TransmitCnt = 0;
            UART_TransmitFlag = 1;
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
    if (GPIO_Pin == USER_Btn_Pin){
        USER_Btn_flag = 1;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if (huart == &huart3){
    	UART_ReceiveFlag = 1;
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
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart3, (uint8_t*)UART_Message, 3); // start receiving info

  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL); // init encoder mode
  __HAL_TIM_SET_COUNTER(&htim1, START_SPEED*ENC_CONST);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // init pwm
  HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1); // sampling from the encoder
  HAL_TIM_Base_Start_IT(&htim6); // init sample time
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, PWM_MIN); // set pwm start value to zero
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){
	  // input via encoder
	  ENC_Cnt = __HAL_TIM_GET_COUNTER(&htim1)/ENC_CONST;
	  if(ENC_Cnt_prev != ENC_Cnt){
		  if(ENC_Cnt >= MAX_SPEED){
			  ENC_Cnt = MAX_SPEED;
			  Pid1.y_ref = MAX_SPEED;
		  }
		  else if(ENC_Cnt <= 0){
			  ENC_Cnt = 0;
			  Pid1.y_ref = 0;
		  }
		  else{
			  Pid1.y_ref = ENC_Cnt;
		  }


		  ENC_Cnt_prev = ENC_Cnt;
	  }

	  // polling

	  if (USER_Btn_flag){
		  USER_Btn_flag = 0;
		  PID_reset(&Pid1);
	  }

	  if(UART_ReceiveFlag){
		UART_ReceiveFlag = 0;
		int rx = atoi(UART_Message);

		if(rx >= MAX_SPEED){
			Pid1.y_ref = MAX_SPEED;
		}
		else{
			Pid1.y_ref = rx;
			__HAL_TIM_SET_COUNTER(&htim1, rx*ENC_CONST);
		}

		HAL_UART_Receive_IT(&huart3, (uint8_t*)UART_Message, 3);
	  }

	  if(UART_TransmitFlag){
		UART_TransmitFlag = 0;
		char tx[32];
		int len = snprintf(tx, sizeof(tx), "%.3f\r\n", Pid1.y);

		HAL_UART_Transmit(&huart3, (uint8_t*)tx, len, 1);
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
