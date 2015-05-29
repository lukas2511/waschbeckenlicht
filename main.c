#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/iwdg.h>

#define LIGHT_TIME 10
#define MAX_DISTANCE 50

void delay_ms(uint32_t ms);

uint32_t echotime;
uint32_t distance;
uint32_t lighttime;

uint32_t delaytime;

void sys_tick_handler(void)
{
	delaytime++;
	echotime++;
	lighttime++;

	if(lighttime > (LIGHT_TIME * 1000000)) {
		lighttime = LIGHT_TIME * 1000000;
	}
}

void delay_ms(uint32_t ms) {
	delaytime = 0;
	while(delaytime < (ms * 1000)) {
		gpio_get(GPIOC, GPIO13);
	}
}

int main(void) {
	/* clock setup */
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* gpio setup */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set(GPIOA, GPIO8);
	gpio_set(GPIOC, GPIO13);

	/* pin C13, buz11 / led */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

	/* pin A8, sonar trig output */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO8);

	/* pin A9, sonar echo input */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_INPUT_FLOAT, GPIO9);

	/* systick setup */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload(9);
	systick_interrupt_enable();
	systick_counter_enable();

	/* watchdog setup */
	iwdg_start();
	iwdg_set_period_ms(1000);

	lighttime = LIGHT_TIME * 1000000;

	delay_ms(50);

	while(1) {
		iwdg_reset();
		gpio_clear(GPIOA, GPIO8); // enable trigger

		while(!gpio_get(GPIOA, GPIO9)); // wait for start
		echotime = 0; // set counter to 0
		while(gpio_get(GPIOA, GPIO9)); // wait for end
		distance = ((echotime / 100) * 172) / 100;

		gpio_set(GPIOA, GPIO8); // disable trigger

		if(distance < MAX_DISTANCE) {
			lighttime = 0;
		}

		if(lighttime < (LIGHT_TIME * 1000000)) {
			gpio_set(GPIOC, GPIO13);
		} else {
			gpio_clear(GPIOC, GPIO13);
		}

		delay_ms(50);
	}

	return 0;
}
