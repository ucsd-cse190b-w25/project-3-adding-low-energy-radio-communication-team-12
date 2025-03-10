#include <i2c.h>

// PB10 and PB11
// Master Mode; > 400khz
void i2c_init() {
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C2EN;;   // Enable I2C2 clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;  // Enable GPIOB clock

    // Set PB10 and PB11 to Alternate Function mode
    GPIOB->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11);  // Clear mode bits
    GPIOB->MODER |= (GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1);  // Set Alternate Function mode

    // Set Alternate Function to I2C2 (AF4 for STM32F4)
    GPIOB->AFR[1] |= (4 << 8) | (4 << 12);

    // Enable Open-Drain Mode for I2C
    GPIOB->OTYPER |= GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11;

    // Set pins to High-Speed Mode
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED10 | GPIO_OSPEEDR_OSPEED11);

    // Enable Pull-Up Resistors
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11);  // Clear previous settings
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0);  // Enable Pull-Up

    //Configuration of I2C2
    I2C2->CR1 &= ~I2C_CR1_PE; //Disables I2C2

    I2C2->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF | I2C_ICR_BERRCF;

    I2C2->TIMINGR = 0x00300F33; // 400 kHz

    I2C2->CR1 |= I2C_CR1_PE; // Enables I2C2 
}

uint8_t i2c_transaction(uint8_t address, uint8_t dir, uint8_t* data, uint8_t len) {

    // Wait until I2C is not busy
    while (I2C2->ISR & I2C_ISR_BUSY);

    //clear bits in CR2
    I2C2->CR2 &= 0;
    // I2C2->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RD_WRN);


    // Configure I2C transaction
    I2C2->CR2 = (address << 1) | (len << 16) | (dir == 1 ? I2C_CR2_RD_WRN : 0);

    // Start Transmission/Reception
    I2C2->CR2 |= I2C_CR2_START;

    if (dir == 0) { // Write Mode
        for (uint8_t i = 0; i < len; i++) {
            // Wait until TX buffer is empty      
            
        	// while (!(I2C2->ISR & I2C_ISR_TXIS)); // or TXIS
            while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)));

            // Write each byte
            I2C2->TXDR = data[i];
        }

        // Wait until transfer is complete
        while (!(I2C2->ISR & I2C_ISR_TC));
        
    } else { // Read Mode
        for (uint8_t i = 0; i < len; i++) {
            // Wait until RX buffer is full
            while (!(I2C2->ISR & I2C_ISR_RXNE));

            // Read each byte
            data[i] = I2C2->RXDR;
        }
    }

    // Stop Condition
    I2C2->CR2 |= I2C_CR2_STOP;

    return 0; // Success
}



