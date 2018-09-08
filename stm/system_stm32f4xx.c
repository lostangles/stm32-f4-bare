#include "stm32f4xx.h"

#if !defined  (HSE_VALUE) 
#define HSE_VALUE    ((uint32_t)25000000) /*!< Default value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
#define HSI_VALUE    ((uint32_t)16000000) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

#define PLL_M 8
#define PLL_N 180
#define PLL_P 0 // 0x00 = PLL_P == 2
#define PLL_Q 4


#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field. 
                                   This value must be a multiple of 0x200. */

uint32_t SystemCoreClock = 16000000;
const uint8_t AHBPrescTable[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9 };
const uint8_t APBPrescTable[8] = { 0, 0, 0, 0, 1, 2, 3, 4 };

void SystemInit(void)
{
	/* FPU settings ------------------------------------------------------------*/
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10 and CP11 Full Access */
#endif
  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set HSION bit */
	RCC->CR |= (uint32_t)0x00000001;

	/* Reset CFGR register */
	RCC->CFGR = 0x00000000;

	/* Reset HSEON, CSSON and PLLON bits */
	RCC->CR &= (uint32_t)0xFEF6FFFF;

	/* Reset PLLCFGR register */
	RCC->PLLCFGR = 0x24003010;

	/* Reset HSEBYP bit */
	RCC->CR &= (uint32_t)0xFFFBFFFF;

	/* Disable all interrupts */
	RCC->CIR = 0x00000000;



	/* Configure the Vector Table location add offset address ------------------*/
#ifdef VECT_TAB_SRAM
	SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
	SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	volatile uint32_t StartUpCounter = 0, HSIStatus = 0;

	//Enable HSI//
	RCC->CR |= RCC_CR_HSION;
	//Wait for HSI to be ready
	do {
		HSIStatus = RCC->CR & RCC_CR_HSIRDY;
		StartUpCounter++;
	} while ((HSIStatus == 0) && (StartUpCounter != 500))
		;
	if ((RCC->CR & RCC_CR_HSIRDY) != 0)
	{
		HSIStatus = 0x01;
	}
	else
	{
		HSIStatus = 0x00;
	}
	if (HSIStatus == 0x01)
	{
		//Enable high performance mode//
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->CR |= PWR_CR_VOS;
		// HCLK = SYSCLK/1
		RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
		// PLCK2 = HCLK / 2
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
		// PLCK1 = HCLK / 4
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

		//Configure the main PLL

		RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (PLL_P << 16) | (PLL_Q << 24);

		//Enable the main PLL
		RCC->CR |= RCC_CR_PLLON;
		//Wait for PLL to turn on
		while((RCC->CR & RCC_CR_PLLRDY) == 0);

		//Set power over drive
		PWR->CR |= 0x01 << 16;
		//Wait for over drive to become ready
		while((PWR->CSR & (0x01 << 16) != 1));
		PWR->CR |= 0x01 << 17;
		while ((PWR->CSR & (0x01 << 17) != 1))
			;

		//Configure Flash
		FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_5WS;

		//Select main PLL as system clock source
		RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
		RCC->CFGR |= RCC_CFGR_SW_PLL;
		//Wait for the main PLL to be used as system clock
		while((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);



	}
	else
	{
		//HSE failed to start up
		 while(1);
	}
	//Enable the CCM RAM clock
	RCC->AHB1ENR |= (1 << 20);
}

void SystemCoreClockUpdate(void)
{
	uint32_t tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;
  
	/* Get SYSCLK source -------------------------------------------------------*/
	tmp = RCC->CFGR & RCC_CFGR_SWS;

	switch (tmp)
	{
	case 0x00:  /* HSI used as system clock source */
		SystemCoreClock = HSI_VALUE;
		break;
	case 0x04:  /* HSE used as system clock source */
		SystemCoreClock = HSE_VALUE;
		break;
	case 0x08:  /* PLL used as system clock source */

	  /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N
	     SYSCLK = PLL_VCO / PLL_P
	     */
		pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
		pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
      
		if (pllsource != 0)
		{
			/* HSE used as PLL clock source */
			pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
		}
		else
		{
			/* HSI used as PLL clock source */
			pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
		}

		pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> 16) + 1) * 2;
		SystemCoreClock = pllvco / pllp;
		break;
	default:
		SystemCoreClock = HSI_VALUE;
		break;
	}
	/* Compute HCLK frequency --------------------------------------------------*/
	/* Get HCLK prescaler */
	tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
	/* HCLK frequency */
	SystemCoreClock >>= tmp;
}
