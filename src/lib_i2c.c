// Author: Farrell Farahbod <farrellfarahbod@gmail.com>
// License: public domain

#include "lib_i2c.h"
#include "stdarg.h"
#include "stm32f429xx.h"

/**
 * Configures the I2C peripheral.
 *
 * @param i2c     I2C1 or I2C2
 * @param speed   STANDARD_MODE_100KHZ or FAST_MODE_400KHZ or FAST_MODE_PLUS_1MHZ
 * @param sck     The pin used for the I2C clock signal
 * @param sda     The pin used for the I2C data signal
 */
void i2c_setup(I2C_TypeDef *i2c, enum I2C_SPEED speed, enum GPIO_PIN sck_pin, enum GPIO_PIN sda_pin) {

	// "unstick" any I2C slave devices that might be in a bad state by driving the clock a few times while SDA is high
	gpio_setup(sck_pin, OUTPUT, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF4);
	gpio_setup(sda_pin, OUTPUT, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF4);
	gpio_high(sda_pin);
	for (uint8_t i = 0; i < 10; i++) {
		gpio_low(sck_pin);
		for (volatile uint32_t j = 0; j < 1000; j++)
			;
		gpio_high(sck_pin);
		for (volatile uint32_t j = 0; j < 1000; j++)
			;
	}

	// configure the GPIOs
	gpio_setup(sck_pin, AF, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF4);
	gpio_setup(sda_pin, AF, OPEN_DRAIN, FIFTY_MHZ, PULL_UP, AF4);

	if (i2c == I2C1) {

		// enable clock, then reset
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;



	}
	else if (i2c == I2C2) {

		// enable clock, then reset
		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
		RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
		RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;

	}


	uint32_t freqrange = 0U;
	uint32_t pclk1 = 0U;
	
	pclk1 = SystemCoreClock >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos];
	freqrange = pclk1 / 1000000U;

	// set timing
	i2c->CR2 |= freqrange;  // APB1 CLK = 45 Mhz
	if(speed == STANDARD_MODE_100KHZ)
	{
		i2c->TRISE = freqrange + 1U;
		i2c->CCR =  (((((pclk1) / ((100000) << 1U)) & I2C_CCR_CCR) < 4U) ? 4U : ((pclk1) / ((100000) << 1U)));
	}
	else if(speed == FAST_MODE_400KHZ)
	{
	
		i2c->TRISE = ((((freqrange) * 300U) / 1000U) + 1U);
		i2c->CCR =   ((pclk1) / ((400000) * 3U));
	}
	// enable
	i2c->CR1 |= 1;

}

/**
 * Writes to one register of an I2C device
 *
 * @param i2c           I2C1 or I2C2
 * @param i2c_address   I2C device address
 * @param reg           Register being written to
 * @param value         Value for the register
 */
void i2c_write_register(I2C_TypeDef *i2c, uint8_t i2c_address, uint8_t reg, uint8_t value) {

	//disable pos
	i2c->CR1 &= ~I2C_CR1_POS;
	// write two bytes with a start bit and a stop bit
	i2c->CR1 |= I2C_CR1_START ;
	while ((i2c->SR1 & I2C_SR1_SB) == 0)
		;
	//send address
	i2c->DR = (i2c_address << 1);
	
	//wait on address flag
	while ((i2c->SR1 & I2C_SR1_ADDR) == 0)
		;
	__I2C_CLEAR_ADDRFLAG(i2c);
	
	//wait for TXE flag
	while((i2c->SR1 & I2C_SR1_TXE) == 0)
		;
	
	//Send reg address
	i2c->DR = reg;
	
	//wait for TXE flag
	while((i2c->SR1 & I2C_SR1_TXE) == 0)
		;
	i2c->DR = value;
	//wait for BTF flag
	while((i2c->SR1 & I2C_SR1_BTF) == 0)
		;
	//generate stop
	i2c->CR1 |= I2C_CR1_STOP;  
}

/**
 * Read the specified number of bytes from an I2C device
 *
 * @param i2c           I2C1 or I2C2
 * @param i2c_address   I2C device address
 * @param byte_count    Number of bytes to read
 * @param first_reg     First register to read from
 * @param rx_buffer     Pointer to an array of uint8_t's where values will be stored
 */
void i2c_read_registers(I2C_TypeDef *i2c, uint8_t i2c_address, uint8_t byte_count, uint8_t first_reg, uint8_t *rx_buffer) {

	uint8_t *ptr = rx_buffer;
	
	//wait for BUSY flag to reset
	while((i2c->SR2 & I2C_SR2_BUSY) == 1)
		;
	
	if (i2c->CR1 & 0x01 != 1)
		i2c->CR1 |= 0x01;
	
	//disable pos
	i2c->CR1 &= ~I2C_CR1_POS;
	
	//enable ack
	i2c->CR1 |= I2C_CR1_ACK;
	//generate start
	i2c->CR1 |= I2C_CR1_START;
	//wait for SB flag
	while ((i2c->SR1 & I2C_SR1_SB) == 0)
		;
	//send slave address
	i2c->DR = (i2c_address << 1) ;
	//wait on address flag
	while((i2c->SR1 & I2C_SR1_ADDR) == 0)
		;
	//Clear address flag by reading SR2
	__I2C_CLEAR_ADDRFLAG(i2c);
	
	//wait for TXE flag
	while((i2c->SR1 & I2C_SR1_TXE) == 0)
		;
	
	//send reg address
	i2c->DR = first_reg;
	
	//wait for TXE flag
	while((i2c->SR1 & I2C_SR1_TXE) == 0)
		;
	
	//generate restart
	i2c->CR1 |= I2C_CR1_START;
	
	//wait for SB flag
	while((i2c->SR1 & I2C_SR1_SB) == 0)
		;
	//send slave address
	i2c->DR = (i2c_address << 1) | 0x01;
	
	//wait on address flag
	while((i2c->SR1 & I2C_SR1_ADDR) == 0)
		;
	//Clear address flag by reading SR2
	__I2C_CLEAR_ADDRFLAG(i2c);

	switch (byte_count)
	{
	case 0: 	
		//Clear address flag by reading SR2
		__I2C_CLEAR_ADDRFLAG(i2c);

		//generate stop
		i2c->CR1 |= I2C_CR1_STOP;  
		break;
	case 1:
		//disable ack
		i2c->CR1 &= I2C_CR1_ACK;
		//Clear address flag by reading SR2
		__I2C_CLEAR_ADDRFLAG(i2c);
		//generate stop
		i2c->CR1 |= I2C_CR1_STOP;  
		break;
	case 2:
		//disable ack
		i2c->CR1 &= I2C_CR1_ACK;
		//enable pos
		i2c->CR1 |= I2C_CR1_POS;
		//clear addr flag
		__I2C_CLEAR_ADDRFLAG(i2c);
		break;
	default:
		//clear addr flag
		__I2C_CLEAR_ADDRFLAG(i2c);
	}
	
	while (byte_count > 0U)
	{
		if (byte_count <= 3U)
		{
			/* One byte */
			if (byte_count == 1U)
			{
				/* Wait until RXNE flag is set */
				while((i2c->SR1 & I2C_SR1_RXNE) == 0)
					;

				/* Read data from DR */
				*rx_buffer++ = i2c->DR;
				byte_count--;
			}
			/* Two bytes */
			else if(byte_count == 2U)
			{
				/* Wait until BTF flag is set */
				//wait for BTF flag
				while((i2c->SR1 & I2C_SR1_BTF) == 0)
					;

				//generate stop
				i2c->CR1 |= I2C_CR1_STOP;  

				/* Read data from DR */
				*rx_buffer++ = i2c->DR;
				byte_count--;

				/* Read data from DR */
				*rx_buffer++ = i2c->DR;
				byte_count--;
			}
			/* 3 Last bytes */
			else
			{
				/* Wait until BTF flag is set */
				while ((i2c->SR1 & I2C_SR1_BTF) == 0)
					;

				//disable ack
				i2c->CR1 &= ~I2C_CR1_ACK;

				/* Read data from DR */
				*rx_buffer++ = i2c->DR;
				byte_count--;

				//wait for BTF flag
				while((i2c->SR1 & I2C_SR1_BTF) == 0)
					;

				//generate stop
				i2c->CR1 |= I2C_CR1_STOP;  

				/* Read data from DR */
				*rx_buffer++ = i2c->DR;
				byte_count--;

				/* Read data from DR */
				*rx_buffer++ = i2c->DR;
				byte_count--;
			}
		}
		else
		{
			/* Wait until RXNE flag is set */
			while ((i2c->SR1 & I2C_SR1_RXNE) == 0)
				;

			/* Read data from DR */
			*rx_buffer++ = i2c->DR;
			byte_count--;

			if (i2c->SR1 & I2C_SR1_BTF == 1)
			{
				/* Read data from DR */
				*rx_buffer++ = i2c->DR;
				byte_count--;
			}
		}
	}
}
