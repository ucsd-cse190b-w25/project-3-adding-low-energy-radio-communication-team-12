/*
 * leds.c
 *
 *  Created on: Oct 3, 2023
 *      Author: schulman
 */


/* Include memory map of our MCU */
#include <stm32l475xx.h>

void leds_init()
{
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

  /* Configure PA5 as an output by clearing all bits and setting the mode */
  GPIOA->MODER &= ~GPIO_MODER_MODE5;
  GPIOA->MODER |= GPIO_MODER_MODE5_0;

  /* Configure the GPIO output as push pull (transistor for high and low) */
  GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;

  /* Disable the internal pull-up and pull-down resistors */
  GPIOA->PUPDR &= GPIO_PUPDR_PUPD5;

  /* Configure the GPIO to use low speed mode */
  GPIOA->OSPEEDR |= (0x3 << GPIO_OSPEEDR_OSPEED5_Pos);

  /* Turn off the LED 1 */
  GPIOA->ODR &= ~GPIO_ODR_OD5;

  // --------------------------- PB14 (LED 2 ) -----------------------------------

  /* Configure PB14 as an output by clearing all bits and setting the mode */
  GPIOB->MODER &= ~GPIO_MODER_MODE14;
  GPIOB->MODER |= GPIO_MODER_MODE14_0;

  /* Configure the GPIO output as push pull (transistor for high and low) */
  GPIOB->OTYPER &= ~GPIO_OTYPER_OT14;

  /* Disable the internal pull-up and pull-down resistors */
  GPIOB->PUPDR &= GPIO_PUPDR_PUPD14;

  /* Configure the GPIO to use low speed mode */
  GPIOB->OSPEEDR |= (0x3 << GPIO_OSPEEDR_OSPEED14_Pos);

  /* Turn off the LED 2 */
  GPIOB->ODR &= ~GPIO_ODR_OD14;
}

void leds_set(uint8_t led)
{
  // Extract relevant bits (First bit for LED1 and second bit for LED2)
  // bit shift right LED2 for ease
  uint8_t led1 = led & (uint8_t)0x01;
  uint8_t led2 = (led & (uint8_t)0x02) >> 1;


  // Turn on or off LED 1 based on extracted bit
  if(led1 == (uint8_t)0x01){

    /* Turn ON the LED1 */
    GPIOA->ODR |= GPIO_ODR_OD5;
  }
  else{

    /* Turn off the LED1 */
    GPIOA->ODR &= ~GPIO_ODR_OD5;
  }


  // Turn on or off LED 2 based on extracted bit
  if(led2 == (uint8_t)0x01){

    /* Turn ON the LED2 */
    GPIOB->ODR |= GPIO_ODR_OD14;
  }
  else{

    /* Turn off the LED2 */
    GPIOB->ODR &= ~GPIO_ODR_OD14;
  }
}
