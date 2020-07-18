#include "ir_decoder.h"
#include <string.h>

uint8_t IR_Is_Repeat(IRSample *sample);
uint8_t IR_Is_Start(IRSample *sample);
uint8_t IR_ReadBit(IRSample *sample, uint32_t* out);

#define IR_ZERO_BIT 0x00000000
#define IR_IS_DURATION(measured, expected) ((measured <= (expected + IR_TOLERANCE_US)) && (measured >= (expected - IR_TOLERANCE_US)))
#define IR_IS_PATTERN(sample, total, on) ((IR_IS_DURATION(sample->impulseDuration, total) && IR_IS_DURATION(sample->impulseOnTime, on)))

#if IR_CODE_LSB_FIRST
#define IR_ONE_BIT 0x80000000
#define IR_APPEND_BIT(val, acc) ((val | acc >> 1))
#else
#define IR_ONE_BIT 0x00000001
#define IR_APPEND_BIT(val, acc) ((val | acc << 1))
#endif

void IR_Reset(IRContext *ctx) {
  memset(ctx, 0, sizeof(IRContext));
}

void IR_Process_Sample(IRContext *ctx) {
  if (!ctx->sample.fresh) return;

  uint32_t inBit;

  if (IR_Is_Repeat(&ctx->sample)) {
    ctx->code.bitsLeft = 0;

    if (ctx->command.status == IR_CMD_RUNNING) {
      ctx->command.status = IR_CMD_REPEAT;
    }
  } if (IR_Is_Start(&ctx->sample)) {
    ctx->code.bitsLeft = IR_CODE_BIT_LENGTH;
    ctx->code.code = 0;
  } else if ((ctx->code.bitsLeft > 0) && IR_ReadBit(&ctx->sample, &inBit)) {
    ctx->code.code = IR_APPEND_BIT(inBit, ctx->code.code);
    ctx->code.bitsLeft--;

    if (!ctx->code.bitsLeft) {
      ctx->command.command = ctx->code.code;
      ctx->command.status = IR_CMD_NEW;
    }
  } else {
    ctx->code.bitsLeft = 0;
  }

  ctx->sample.fresh = 0;
}

uint8_t IR_NEC_Decode(uint32_t command, uint8_t *addr, uint8_t *cmd) {
  *cmd = (command >> 16) & 0xff;
  *addr = command & 0xff;

  return (*cmd == (~(command >> 24) & 0xff)) && (*addr == (~(command >> 8) & 0xff));
}

inline uint8_t IR_Is_Repeat(IRSample *sample) {
  return IR_IS_PATTERN(sample, IR_REPEAT_DURATION_US, IR_REPEAT_ON_US);
}

inline uint8_t IR_Is_Start(IRSample *sample) {
  return IR_IS_PATTERN(sample, IR_START_DURATION_US, IR_START_ON_US);
}

uint8_t IR_ReadBit(IRSample *sample, uint32_t* out) {
  if (IR_IS_PATTERN(sample, IR_ZERO_DURATION_US, IR_BIT_ZERO_ON_US)) {
    *out = IR_ZERO_BIT;
  } else if (IR_IS_PATTERN(sample, IR_ONE_DURATION_US, IR_BIT_ONE_ON_US)) {
    *out = IR_ONE_BIT;
  } else {
    return 0;
  }

  return 1;
}