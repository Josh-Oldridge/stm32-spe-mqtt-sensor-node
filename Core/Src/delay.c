#include "delay.h"
#include "stm32l4xx.h"  // Needed for SystemCoreClock and __NOP()

/**
  * @brief  Delay for a specified number of microseconds.
  * @param  us: the number of microseconds to delay.
  * @note   The loop constant is an approximation and may need calibration for your system clock.
  */
void DelayMicroseconds(uint32_t us)
{
    /* This is a rough approximation.
       You may need to calibrate the divisor (here, 5) based on your SystemCoreClock.
       For example, if SystemCoreClock is 80 MHz, then:
           SystemCoreClock / 1000000U == 80
       Adjust the divisor so that the delay matches your expectation.
    */
    volatile uint32_t count = (SystemCoreClock / 1000000U) * us / 5U;
    while (count--) {
        __NOP();
    }
}
