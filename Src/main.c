#include "main.h"
#include "ir_decoder.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim14;

IRContext irContext;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM14_Init(void);
static void Sleep(void);
static void ProcessCommand(IRCommand *cmd);

int main(void) {
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM14_Init();

  IR_Reset(&irContext);

  while (1)  {
    Sleep();
    IR_Process_Sample(&irContext);
    ProcessCommand(&irContext.command);
  }
}

void ProcessCommand(IRCommand *cmd) {
  if (cmd->status != IR_CMD_NEW) return;
  cmd->status = IR_CMD_RUNNING;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
    irContext.sample.impulseDuration = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    irContext.sample.fresh = 1;
  } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    irContext.sample.impulseOnTime = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
  }
}

static void Sleep(void) {
  HAL_SuspendTick();
  __WFI();
  HAL_ResumeTick();
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_TIM1_Init(void) {
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = (uint16_t) (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xffff;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }

  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI2FP2;
  sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sSlaveConfig.TriggerPrescaler = TIM_ICPSC_DIV1;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim1, &sSlaveConfig) != HAL_OK) {
    Error_Handler();
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  if (HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }

	if (HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
}

static void MX_TIM14_Init(void) {
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim14.Instance = TIM14;
  htim14.Init.Prescaler = (uint16_t) (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 13500;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)  {
    Error_Handler();
  }

  if (HAL_TIM_PWM_Init(&htim14) != HAL_OK) {
    Error_Handler();
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 9000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim14);

  /*if (HAL_TIM_PWM_Start_IT(&htim14, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }*/
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void) {

}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {

}
#endif
