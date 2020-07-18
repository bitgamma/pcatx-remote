#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"

void Error_Handler(void);

#define PWR_BTN_Pin GPIO_PIN_4
#define PWR_BTN_GPIO_Port GPIOA

#ifdef __cplusplus
}
#endif

#endif
