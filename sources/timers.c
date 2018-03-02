#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include "timers.h"
#include "usart.h"
extern uint8_t	upcount;
extern uint16_t compare_time;
/*
 *Input capture timer
 */
void tim2_init(void)
{
	/* Set up the timer TIM2 for injected sampling */
	uint32_t timer;

	rcc_periph_clock_enable(RCC_TIM2);

	/* Time Base configuration */
	rcc_periph_reset_pulse(RST_TIM2);
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
			TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_period(TIM2, 0xFF);
	timer_set_prescaler(TIM2, 0x8);
	timer_set_clock_division(TIM2, 0x0);
	/* Generate TRGO on every update. */
	timer_set_master_mode(TIM2, TIM_CR2_MMS_UPDATE);
	timer_enable_counter(TIM2);
} 
/*
 *PWM timer
 *Freq: 39 KHz
 *we are using a low level on output pin to drive a MOSFET,
 so PWM is configured with output polarity low.
 */
void tim1_init(void)
{

	/* Enable TIM1 clock. */
	rcc_periph_clock_enable(RCC_TIM1);
	/* Reset TIM1 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM1);
	timer_set_period(TIM1, TIMER1_TOP);


	 /*
	  *Timer global mode:
	  *- No divider
	  *- Alignment edge
	  *- Direction up
	  *(These are actually default values after reset above, so this call
	  *is strictly unnecessary, but demos the api for alternative settings)
	  */
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT,
			TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
timer_enable_break_main_output(TIM1);
	 /*
	  *Please take note that the clock source for STM32 timers
	  *might not be the raw APB1/APB2 clocks.  In various conditions they
	  *are doubled.  See the Reference Manual for full details!
	  *In our case, TIM2 on APB1 is running at double frequency, so this
	  *sets the prescaler to have the timer run at 5kHz
	  */
	timer_set_prescaler(TIM1, 0);

	/* Enable preload. */
	timer_enable_preload(TIM1);
	timer_continuous_mode(TIM1);
	timer_enable_oc_preload(TIM1,TIM_OC1);
	timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
	/* Period (39kHz). */
	timer_set_period(TIM1, 72000000 / 39000);
	/*Output compare polarity*/
	timer_set_oc_polarity_low(TIM1, TIM_OC1);

	/* Set the initual output compare value for OC1. */
	tim1_set_pwm(START_PWM_VALUE);
	/*tim1_set_pwm(1000);*/
	/*timer_set_oc_value(TIM1, TIM_OC1, 1600);*/
	/* Enable TIM1 interrupt. */
	nvic_enable_irq(NVIC_TIM1_CC_IRQ);
	nvic_enable_irq(NVIC_TIM1_UP_IRQ);

	timer_enable_oc_output(TIM1, TIM_OC1);

	/*Enable timer 1 overflow and compare int */
	timer_enable_irq(TIM1, (TIM_DIER_UIE));
	timer_enable_irq(TIM1, (TIM_DIER_CC1IE));
		timer_enable_counter(TIM1);
}

void tim1_enable(uint8_t param)
{
	if (param == true)
		timer_enable_counter(TIM1);
	else if (param == false)
		timer_disable_counter(TIM1);
}

/* @brief Sets PWM duty
 * @param PWM duty in percents
 * */
void tim1_set_pwm (uint8_t pwm)
{
	uint16_t compareVal;
	/*usart_send_byte(USART1, pwm);*/
	compareVal = (uint16_t)(TIMER1_TOP * pwm / 100);
	timer_set_oc_value(TIM1, TIM_OC1, compareVal); 
	/*usart_send_string(USART1, "PWM updated\n", strlen("PWM updated\n"));*/
}
void tim1_up_isr(void)
{
	/* Clear update interrupt flag. */
	timer_clear_flag(TIM1, TIM_SR_UIF);
	gpio_set(GREEN_LED_PORT, GREEN_LED);
}
void tim1_cc_isr (void)
{
	/* Clear compare interrupt flag. */
	timer_clear_flag(TIM1, TIM_SR_CC1IF);

	gpio_clear(GREEN_LED_PORT, GREEN_LED);

/*
 *        if (upcount ==1){
 *                compare_time += COMPARE_STEP;
 *        }else{
 *                compare_time -= COMPARE_STEP;
 *        }
 *
 *        if (compare_time == 59000){
 *                upcount = 0;
 *        }
 *        if (compare_time == 0){
 *                upcount = 1;
 *        }
 *        timer_set_oc_value(TIM1, TIM_OC1, compare_time); 
 */
}

/*
 *Timer 3 used to generate single impulse to motor breaking circuit
 *Lenth of breaking impulse defines with BREAK_IMPULSE_LENTH (in ms)
 */
void tim3_init(void)
{
	/* Enable TIM1 clock. */
	rcc_periph_clock_enable(RCC_TIM3);
	/* Reset TIM1 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM3);


	/* Timer global mode:
	 * - No divider
	 * - Alignment edge
	 * - Direction up
	 * (These are actually default values after reset above, so this call
	 * is strictly unnecessary, but demos the api for alternative settings)
	 */
	timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT,
			TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*Set cycle duty to 3 seconds*/
	timer_set_prescaler(TIM3, 60000-1);
	timer_set_period(TIM3, TIMER3_TOP);

	/* Disable preload. */
	timer_disable_preload(TIM3);
	timer_continuous_mode(TIM3);
	timer_enable_oc_preload(TIM3,TIM_OC3);
	/*Output compare polarity*/
	/*timer_set_oc_polarity_low(TIM3, TIM_OC3);*/

	/* Set the initual output compare value for OC1. */
	/*tim1_set_pwm(BREAK_IMPULSE_LENTH);*/
	/*timer_set_oc_value(TIM3, TIM_OC1, BREAK_IMPULSE_LENTH); */

	/* Enable TIM3 interrupt. */
	/*nvic_enable_irq(NVIC_TIM3_CC_IRQ);*/
	/*nvic_enable_irq(NVIC_TIM3_UP_IRQ);*/

	/*Enable timer 3 overflow and compare int */
	/*timer_enable_irq(TIM3, (TIM_DIER_UIE));*/
	timer_enable_irq(TIM3, (TIM_DIER_CC1IE));

}
