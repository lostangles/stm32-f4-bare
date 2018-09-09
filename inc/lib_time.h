#pragma once

#include <stdint.h>
#include "stm32f429xx.h"
uint32_t maxUsSleep;
uint32_t cyclesPerUs;
extern uint32_t SystemCoreClock;

void EnableCycles();
void sleepUs(uint32_t uS);
void sleepMs(uint32_t mS);
