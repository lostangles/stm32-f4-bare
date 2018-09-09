#include "lib_time.h"

void EnableCycles()
{
	//Uses Cortex-M debugger to count raw CPU cycles
	//Used as free running counter, counts up to 32bit
	//Worth of cycles before looping back around to 0
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	//Number of clock cycles that can fit in DWT->CYCNT (uint_32t) is 
	//4294967296 and it takes (SystemCoreClock / 1000000) cycles for
	//one uS to pass.
	cyclesPerUs = SystemCoreClock / 1000000;
	maxUsSleep = 4294967296 / cyclesPerUs;
}

void sleepUs(uint32_t uS)
{
	volatile uint32_t elapsedUs;
	//If requested sleep is more than the max,
	//set it as max and give it some buffer
	if (uS > maxUsSleep) uS = maxUsSleep - 100;
	elapsedUs = DWT->CYCCNT = 0;
	while (uS > elapsedUs)
	{
		elapsedUs = DWT->CYCCNT / cyclesPerUs;
	}
}

void sleepMs(uint32_t mS)
{
	uint32_t elapsedMs;
	elapsedMs = 0;
	while (elapsedMs++ < mS)
	{
		sleepUs(1000);
	}
}
