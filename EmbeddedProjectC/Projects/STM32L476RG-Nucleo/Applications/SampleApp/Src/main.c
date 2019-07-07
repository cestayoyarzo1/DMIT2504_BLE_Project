/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#define SYSCLK 80000000

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_x-cube-ble1.h"
#include "robot.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  //HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  //SystemClock_Config();
  Clock();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_BlueNRG_MS_Init();
  
  /* USER CODE BEGIN 2 */
  Timers();
  Robot_Init(TIM3, TIM4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

  MX_BlueNRG_MS_Process();
    /* USER CODE BEGIN 3 */
  Robot_Run();
  }
  /* USER CODE END 3 */
}
//CARLOS ADDED CODE
//-------------------------------------------------------------------------clock
void Clock()
{
  //Enable peripherals
  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;  //Enable power interface
  
  //Enable instruction prefetch, data and instruction cache
  FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
  
  //Slowdown FLASH
  FLASH->ACR &= ~FLASH_ACR_LATENCY;
  FLASH->ACR |= FLASH_ACR_LATENCY_1WS;
  
  //System will be running from MSI trimmed from LSE
  
  //Configure LSE
  PWR->CR1 |= PWR_CR1_DBP;                      //Enable backup domain write
  
  RCC->BDCR &= ~RCC_BDCR_BDRST;                 //Do NOT reset backup domain
  RCC->BDCR &= ~RCC_BDCR_LSEBYP;                //No LSE Bypass (use crystal)
  RCC->BDCR |= RCC_BDCR_LSEON;                  //LSE ON
  while(!(RCC->BDCR & RCC_BDCR_LSERDY));        //Wait until LSE is Ready
  
  RCC->BDCR |= RCC_BDCR_RTCSEL_0;             //LSE as RTC clock
  RCC->BDCR |= RCC_BDCR_RTCEN;                //Enable RTC
  
  PWR->CR1 &= ~PWR_CR1_DBP;                     //Disable backup domain write
  
  //Configure MSI
  RCC->CR &= ~RCC_CR_MSION;             //Turn MSI Off
  RCC->CR &= ~RCC_CR_MSIPLLEN;          //Disable MSI PLL
  RCC->CR |= RCC_CR_MSIRGSEL;           //Take Frequency Range from RCC->CR  
  RCC->CR &= ~RCC_CR_MSIRANGE;          //Clear Range
  RCC->CR |= RCC_CR_MSIRANGE_6;        //4Mhz Range
  RCC->CR |= RCC_CR_MSIPLLEN;           //Enable MSI PLL (Auto trim by LSE)
  RCC->CR |= RCC_CR_MSION;              //Turn On MSI
  while(!(RCC->CR & RCC_CR_MSIRDY));    //Wait until MSI is stable
  
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_MSI; //MSI as PLL clock Source
  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;    //Clear PLLM, M = 1
  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;     //Clear PLLN
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLN_3 | RCC_PLLCFGR_PLLN_5; //N = 40
                                        // PLLR is 2 after reset
  
  RCC->CFGR &= ~RCC_CFGR_MCOSEL;        //Clear MCOSEL
  //RCC->CFGR |= RCC_CFGR_MCOSEL_1;     //MSI as MCO
  RCC->CFGR |= RCC_CFGR_MCOSEL_0;       //SysCLK as MCO = 80 Mhz
  RCC->CFGR |= RCC_CFGR_MCO_PRE_16;       // MCO / 16 = 5 Mhz
    
  /*
  // USB clocks begin
  RCC->PLLSAI1CFGR = 0x00101800;        //TODO Expand this
  RCC->CR |= RCC_CR_PLLSAI1ON;          //Turn On PLLSAI1
  while(!(RCC->CR & RCC_CR_PLLSAI1RDY));//Wait until PLLSAI1 is stable
  RCC->CCIPR |= RCC_CCIPR_CLK48SEL_0;
  // USB clocks end
  */
  
  RCC->CR |= RCC_CR_PLLON;              //Turn On PLL
  while(!RCC->CR & RCC_CR_PLLRDY);      //Wait until PLL is locked
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;   //Enable PLL Output
  
  RCC->CFGR &= ~RCC_CFGR_SW;            //Clear SySClk selection (fallback to MSI)
  RCC->CFGR |= RCC_CFGR_SW_0 | RCC_CFGR_SW_1; //PLL as SYSCLK
  
  SysTick_Config(SYSCLK / 1000); // 80Mhz / 1000 = 80Khz, 1ms Ticks
  NVIC_EnableIRQ(SysTick_IRQn);
  
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;  //Enable PORT A
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;  //Enable PORT B
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;  //Enable PORT C
  
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // Enable UART 1
  RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN; // Enable UART 2
  RCC->APB1ENR1 |= RCC_APB1ENR1_USART3EN; // Enable UART 3
  
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN; // Enable TIMER 3
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN; // Enable TIMER 4
  
  
  
  /*
  GPIOC->MODER &= ~GPIO_MODER_MODER0;
  GPIOC->MODER |= GPIO_MODER_MODER0_0;
  //GPIOC->BSRR |= GPIO_BSRR_BS_0;
  GPIOC->BRR |= GPIO_BRR_BR_0;
  */
  
  GPIOC->MODER &= ~GPIO_MODER_MODER0;
  GPIOC->MODER |= GPIO_MODER_MODER0;
  //GPIOC->BSRR |= GPIO_BSRR_BS_0;
  //GPIOC->BRR |= GPIO_BRR_BR_0;
}

//Timers------------------------------------------------------------------------
void Timers()
{
  //TIM3 Configuration, right wheels
  
  //PC8->TIM3_CH3 (AF2) -> FORWARD
  GPIOC->MODER &= ~GPIO_MODER_MODER8;           // Clear MODER settings
  GPIOC->MODER |= GPIO_MODER_MODER8_1;          //Alternate function
  GPIOC->AFR[1] |= 2<<0;                        //AF2
  
  //PC9->TIM3_CH4 (AF2) -> BACKWARD
  GPIOC->MODER &= ~GPIO_MODER_MODER9;           // Clear MODER settings
  GPIOC->MODER |= GPIO_MODER_MODER9_1;          //Alternate function
  GPIOC->AFR[1] |= 2<<4;                        //AF2
  
  //Timer Configuration
  
  TIM3->CR1 &= ~TIM_CR1_CEN;            // Stop Timer
  TIM3->PSC = 40-1;                     // Prescale clock to 2Mhz freq.
  TIM3->ARR = 1000;                     // PWM frequency 2Mhz / 1000 (period) = 2Khz
  TIM3->CCR3 = 250;                       // 50% Duty cycle, pwm off.
  TIM3->CCR4 = 250;                       // 50% Duty cycle, pwm off.
  TIM3->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1; //PWM type 1 CH3 (not inverted)
  TIM3->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1; //PWM type 1 CH4 (not inverted)
  TIM3->CCMR2 |= TIM_CCMR2_OC3PE;      //Enable preload for channel 3
  TIM3->CCMR2 |= TIM_CCMR2_OC4PE;      //Enable preload for channel 4
  TIM3->CR1 |= TIM_CR1_ARPE;           //TIM3 ARR register is buffered
  TIM3->CCER |= TIM_CCER_CC3E;         //Enable pwm output 1
  TIM3->CCER |= TIM_CCER_CC4E;         //Enable pwm output 2
  TIM3->BDTR |= TIM_BDTR_MOE;          //Main output enable
  
  TIM3->EGR |= TIM_EGR_UG;            // Enable register update
  TIM3->CR1 |= TIM_CR1_CEN;            // Start timer  
  
  //TIM4 Configuration, left wheels
  
  //PB8->TIM4_CH3 (AF2) -> FORWARD
  GPIOB->MODER &= ~GPIO_MODER_MODER8;           // Clear MODER settings
  GPIOB->MODER |= GPIO_MODER_MODER8_1;          //Alternate function
  GPIOB->AFR[1] |= 2<<0;                        //AF2
 
  //PB9->TIM4_CH4 (AF2) -> BACKWARD
  GPIOB->MODER &= ~GPIO_MODER_MODER9;           // Clear MODER settings
  GPIOB->MODER |= GPIO_MODER_MODER9_1;          //Alternate function
  GPIOB->AFR[1] |= 2<<4;                        //AF2 
  
  //Timer Configuration
  
  TIM4->CR1 &= ~TIM_CR1_CEN;            // Stop Timer
  TIM4->PSC = 40-1;                     // Prescale clock to 2Mhz freq.
  TIM4->ARR = 1000;                     // PWM frequency 2Mhz / 1000 (period) = 2Khz
  TIM4->CCR3 = 500;                       // 50% Duty cycle, pwm off.
  TIM4->CCR4 = 500;                       // 50% Duty cycle, pwm off.
  TIM4->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1; //PWM type 1 CH3 (not inverted)
  TIM4->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1; //PWM type 1 CH4 (not inverted)
  TIM4->CCMR2 |= TIM_CCMR2_OC3PE;      //Enable preload for channel 3
  TIM4->CCMR2 |= TIM_CCMR2_OC4PE;      //Enable preload for channel 4
  TIM4->CR1 |= TIM_CR1_ARPE;           //TIM4 ARR register is buffered
  TIM4->CCER |= TIM_CCER_CC3E;         //Enable pwm output 1
  TIM4->CCER |= TIM_CCER_CC4E;         //Enable pwm output 2
  TIM4->BDTR |= TIM_BDTR_MOE;          //Main output enable  
  
  TIM4->EGR |= TIM_EGR_UG;            // Enable register update
  TIM4->CR1 |= TIM_CR1_CEN;            // Start timer  
  
}
//CARLOS ADDED CODE END*****

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

//  /** Initializes the CPU, AHB and APB busses clocks 
//  */
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
//  RCC_OscInitStruct.PLL.PLLM = 1;
//  RCC_OscInitStruct.PLL.PLLN = 10;
//  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
//  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
//  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|LD2_Pin|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 LD2_Pin PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|LD2_Pin|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
