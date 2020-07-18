#include "main.h"
#include "stm32f0xx_it.h"

extern TIM_HandleTypeDef htim3;

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
  while (1) {

  }
}

void SVC_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
  HAL_IncTick();
}

void TIM3_IRQHandler(void) {
  HAL_TIM_IRQHandler(&htim3);
}
