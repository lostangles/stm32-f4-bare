// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include "lib_exti.h"
#include "lib_gpio.h"
#include "stm32f429xx.h"

// array of event handler function pointers
static void(*exti_handler[16])(void) = { 0 };

/**
 * Configures an external interrupt.
 * EXTI0 can be pin 0 of any gpio port, EXTI1 can be pin 1 of any gpio port, etc.
 * You can not have interrupts for two pins with the same number but on different ports.
 *
 * @param pin       GPIO pin used as an external interrupt
 * @param pull      NO_PULL or PULL_UP or PULL_DOWN
 * @param edge      RISING_EDGE or FALLING_EDGE or BOTH_EDGES
 * @param handler   Pointer to an event handler function
 */
void exti_setup(enum GPIO_PIN pin, enum GPIO_PULL pull, enum EXTI_EDGE edge, void(*handler)(void)) {

	uint8_t pinNumer = pin % 16;
	char port = (pin / 16);  // 0 means GPIOA, etc.
	
	gpio_setup(pin, INPUT, PUSH_PULL, FIFTY_MHZ, pull, AF0);

	exti_handler[pinNumer] = handler;

	EXTI->IMR |= (1 << pinNumer);  // unmask exti

	if((edge == RISING_EDGE) || (edge == BOTH_EDGES))	EXTI->RTSR |= (1 << pinNumer);  // trigger exti on rising edges
	if((edge == FALLING_EDGE) || (edge == BOTH_EDGES))	EXTI->FTSR |= (1 << pinNumer);  // trigger exti on falling edges

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	// specify which port the exti is connected to
	uint32_t temp = SYSCFG->EXTICR[pinNumer / 4];
	switch (port) {
	case 0: // A
		// bits = 0000
		temp &= ~(1 << (pinNumer % 4 * 4) + 3);
		temp &= ~(1 << (pinNumer % 4 * 4) + 2);
		temp &= ~(1 << (pinNumer % 4 * 4) + 1);
		temp &= ~(1 << (pinNumer % 4 * 4) + 0);
		break;
	case 1: // B
		// bits = 0001
		temp &= ~(1 << (pinNumer % 4 * 4) + 3);
		temp &= ~(1 << (pinNumer % 4 * 4) + 2);
		temp &= ~(1 << (pinNumer % 4 * 4) + 1);
		temp |=  (1 << (pinNumer % 4 * 4) + 0);
		break;
	case 2: // C
		// bits = 0010
		temp &= ~(1 << (pinNumer % 4 * 4) + 3);
		temp &= ~(1 << (pinNumer % 4 * 4) + 2);
		temp |=  (1 << (pinNumer % 4 * 4) + 1);
		temp &= ~(1 << (pinNumer % 4 * 4) + 0);
		break;
	case 3: // D
		// bits = 0011
		temp &= ~(1 << (pinNumer % 4 * 4) + 3);
		temp &= ~(1 << (pinNumer % 4 * 4) + 2);
		temp |=  (1 << (pinNumer % 4 * 4) + 1);
		temp |=  (1 << (pinNumer % 4 * 4) + 0);
		break;
	case 5: // F
		// bits = 0101
		temp &= ~(1 << (pinNumer % 4 * 4) + 3);
		temp |=  (1 << (pinNumer % 4 * 4) + 2);
		temp &= ~(1 << (pinNumer % 4 * 4) + 1);
		temp |=  (1 << (pinNumer % 4 * 4) + 0);
		break;
	}
	SYSCFG->EXTICR[pinNumer / 4] = temp;  // EXTI0=PA0, EXTI1=PA1, EXTI2=PA2, EXTI3=PA3
	

	switch(pinNumer)
	{
	case 0: NVIC_EnableIRQ(EXTI0_IRQn); break;
	case 1: NVIC_EnableIRQ(EXTI1_IRQn); break;
	case 2: NVIC_EnableIRQ(EXTI2_IRQn); break;
	case 3: NVIC_EnableIRQ(EXTI3_IRQn); break;
	case 4: NVIC_EnableIRQ(EXTI4_IRQn); break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9: NVIC_EnableIRQ(EXTI9_5_IRQn); break;
	case 10:
	case 11:
	case 12: 
	case 13:
	case 14:
	case 15: NVIC_EnableIRQ(EXTI15_10_IRQn); break;
	default: break;
		
	}

}

/**
 * Manually trigger an external interrupt.
 *
 * @param pin		GPIO pin associated with the interrupt.
 */
void exti_trigger(enum GPIO_PIN pin) {
	EXTI->SWIER = (1 << (pin % 16));  // trigger exti
}

/**
 * ISR for External Interrupts 0 and 1. Clears the interrupts and calls the handlers.
 */

void  EXTI0_IRQHandler() {

	if (EXTI->PR & EXTI_PR_PR0) {

		EXTI->PR = EXTI_PR_PR0;
		if (exti_handler[0]) exti_handler[0]();

	}
}

void  EXTI1_IRQHandler() {

	if (EXTI->PR & EXTI_PR_PR1) {

		EXTI->PR = EXTI_PR_PR1;
		if (exti_handler[1]) exti_handler[1]();

	}
}

void  EXTI2_IRQHandler() {
	if (EXTI->PR & EXTI_PR_PR2) {

		EXTI->PR = EXTI_PR_PR2;
		if (exti_handler[2]) exti_handler[2]();

	}
}

void  EXTI3_IRQHandler() {
	if (EXTI->PR & EXTI_PR_PR3) {

		EXTI->PR = EXTI_PR_PR3;
		if (exti_handler[3]) exti_handler[0]();

	}
}

void EXTI4_IRQHandler() {
	if (EXTI->PR & EXTI_PR_PR4) {

		EXTI->PR = EXTI_PR_PR4;
		if (exti_handler[4]) exti_handler[4]();

	}
}

void EXTI9_5_IRQHandler() {
	if (EXTI->PR & EXTI_PR_PR5) {

		EXTI->PR = EXTI_PR_PR5;
		if (exti_handler[5]) exti_handler[5]();

	}
	else if (EXTI->PR & EXTI_PR_PR6) {

		EXTI->PR = EXTI_PR_PR6;
		if (exti_handler[6]) exti_handler[6]();

	}
	else if (EXTI->PR & EXTI_PR_PR7) {

		EXTI->PR = EXTI_PR_PR7;
		if (exti_handler[7]) exti_handler[7]();

	}
	else if (EXTI->PR & EXTI_PR_PR8) {

		EXTI->PR = EXTI_PR_PR8;
		if (exti_handler[8]) exti_handler[8]();

	}
	else if (EXTI->PR & EXTI_PR_PR9) {

		EXTI->PR = EXTI_PR_PR9;
		if (exti_handler[9]) exti_handler[9]();
	}
}

void EXTI15_10_IRQHandler() {
	if (EXTI->PR & EXTI_PR_PR10) {

		EXTI->PR = EXTI_PR_PR10;
		if (exti_handler[10]) exti_handler[10]();

	}
	else if (EXTI->PR & EXTI_PR_PR11) {

		EXTI->PR = EXTI_PR_PR11;
		if (exti_handler[11]) exti_handler[11]();

	}
	else if (EXTI->PR & EXTI_PR_PR12) {

		EXTI->PR = EXTI_PR_PR12;
		if (exti_handler[12]) exti_handler[12]();

	}
	else if (EXTI->PR & EXTI_PR_PR13) {

		EXTI->PR = EXTI_PR_PR13;
		if (exti_handler[13]) exti_handler[13]();

	}
	else if (EXTI->PR & EXTI_PR_PR14) {

		EXTI->PR = EXTI_PR_PR14;
		if (exti_handler[14]) exti_handler[14]();

	}
	else if (EXTI->PR & EXTI_PR_PR15) {

		EXTI->PR = EXTI_PR_PR15;
		if (exti_handler[15]) exti_handler[15]();

	}
}
