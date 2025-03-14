/*
 * timer.c
 *
 *  Created on: Oct 5, 2023
 *      Author: schulman
 */

#include "timer.h"


void timer_init(TIM_TypeDef* timer)
{
  // Stop the counter
  timer->CR1 &= ~TIM_CR1_CEN;

  // Set reset bit and clear it
  RCC->APB1RSTR1 |= RCC_APB1RSTR1_TIM2RST;
  RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_TIM2RST;
  
  //Enable the timer clock.
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

  // Set ARPE bit to wait for update

  timer->CR1 |= TIM_CR1_ARPE;

  // Enable the timer’s interrupt both internally and in the interrupt controller (NVIC).

  timer->DIER |= TIM_DIER_UIE;

  NVIC_EnableIRQ(TIM2_IRQn);
  NVIC_SetPriority(TIM2_IRQn, 1);
  
  //Set Prescaler and ARR

  timer->PSC = 999;

  timer->ARR = 9999;

  //Start Counter
  timer->CR1 |= TIM_CR1_CEN;

  //Enable interrupts on system, if not already true
  __enable_irq();

  //Force an update
  timer->EGR |= TIM_EGR_UG;

  // ! Call timer_reset?


}

void timer_reset(TIM_TypeDef* timer)
{
  timer->CNT = 0;
}

void timer_set_ms(TIM_TypeDef* timer, uint16_t period_ms)
{
  timer->ARR = period_ms - 1;
}



