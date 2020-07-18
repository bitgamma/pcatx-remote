#ifndef __IR_DECODER_H
#define __IR_DECODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define IR_CODE_BIT_LENGTH 32;
#define IR_CODE_LSB_FIRST 1

#define IR_TOLERANCE_US 50

#define IR_REPEAT_DURATION_US 11250
#define IR_START_DURATION_US 13500
#define IR_ONE_DURATION_US 2250
#define IR_ZERO_DURATION_US 1125

#define IR_HEADER_ON_US 9000
#define IR_BIT_ON_US 560

typedef struct {
  uint32_t impulseDuration;
  uint32_t impulseOnTime;
  uint8_t fresh;
} IRSample;

typedef struct {
  uint32_t command;
  uint8_t status;
} IRCommand;

typedef struct {
  uint32_t code;
  uint32_t bitsLeft;
} IRCode;

typedef enum {
  IR_CMD_DONE,
  IR_CMD_RUNNING,
  IR_CMD_NEW
} IRCommandStatus;

typedef struct {
  IRSample sample;
  IRCode code;
  IRCommand command;
} IRContext;

void IR_Reset(IRContext *ctx);
void IR_Process_Sample(IRContext *ctx);

#ifdef __cplusplus
}
#endif

#endif