#include <px4_config.h>
#include <px4_time.h>
#include <nuttx/arch.h>
#include <nuttx/irq.h>

#include <sys/types.h>
#include <stdbool.h>

#include <assert.h>
#include <debug.h>
#include <time.h>
#include <queue.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <board_config.h>
#include <drivers/drv_pwm_input.h>

#include <drivers/drv_hrt.h>
#include <drivers/drv_range_finder.h>

#include "chip.h"
#include "up_internal.h"
#include "up_arch.h"

#include "stm32_gpio.h"
#include "stm32_tim.h"
#include <systemlib/err.h>

#include <uORB/uORB.h>
#include <uORB/topics/pwm_input.h>

#include <drivers/drv_device.h>
#include <drivers/device/device.h>
#include <drivers/device/ringbuffer.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if HRT_TIMER == PWMIN_TIMER
#error cannot share timer between HRT and PWMIN
#endif

#if !defined(GPIO_PWM_IN) || !defined(PWMIN_TIMER) || !defined(PWMIN_TIMER_CHANNEL)
#error PWMIN defines are needed in board_config.h for this board
#endif

#define PWMIN_TIMER_BASE	STM32_TIM4_BASE
#define PWMIN_TIMER_POWER_REG	STM32_RCC_APB1ENR
#define PWMIN_TIMER_POWER_BIT 	RCC_APB1ENR_TIM4EN
#define PWMIN_TIMER_VECTOR	STM32_IRQ_TIM4
#define PWMIN_TIMER_CLOCK	STM32_APB1_TIM4_CLKIN

#if PWMIN_TIMER_CLOCK <= 1000000
#error PWMIN_TIMER_CLOCK must be greater than 1MHz
#endif

#define REG(_reg)	(*(volatile uint32_t *)(PWMIN_TIMER_BASE + _reg))

#define rCR1		REG(STM32_GTIM_CR1_OFFSET)
#define rCR2		REG(STM32_GTIM_CR2_OFFSET)
#define rSMCR		REG(STM32_GTIM_SMCR_OFFSET)
#define rDIER		REG(STM32_GTIM_DIER_OFFSET)
#define rSR		REG(STM32_GTIM_SR_OFFSET)
#define rEGR		REG(STM32_GTIM_EGR_OFFSET)
#define rCCMR1		REG(STM32_GTIM_CCMR1_OFFSET)
#define rCCMR2		REG(STM32_GTIM_CCMR2_OFFSET)
#define rCCER		REG(STM32_GTIM_CCER_OFFSET)
#define rCNT		REG(STM32_GTIM_CNT_OFFSET)
#define rPSC		REG(STM32_GTIM_PSC_OFFSET)
#define rARR		REG(STM32_GTIM_ARR_OFFSET)
#define rCCR1		REG(STM32_GTIM_CCR1_OFFSET)
#define rCCR2		REG(STM32_GTIM_CCR2_OFFSET)
#define rCCR3		REG(STM32_GTIM_CCR3_OFFSET)
#define rCCR4		REG(STM32_GTIM_CCR4_OFFSET)
#define rDCR		REG(STM32_GTIM_DCR_OFFSET)
#define rDMAR		REG(STM32_GTIM_DMAR_OFFSET)

extern"C" __EXPORT int publisher_main(int argc, char *argv[]);
__EXPORT int tim4_main(int argc, char *argv[]);

static void tim4_info(void)
{
	PX4_INFO("CLOCK and TIMER SETTING\n");
	printf("TIMER(APB1) CLOCK: %lu", PWMIN_TIMER_CLOCK);
			
	exit(0);
}

static void tim4_usage()
{
	PX4_ERR("unrecognized command, try 'info', 'start', 'test'");
}

int tim4_main(int argc, char *argv[])
{
	if(argc <2){
		tim4_usage();
		return -1;
	}

	const char *verb = argv[1];

	if (!strcmp(verb, "info")) {
		tim4_info();
	}
/*
	if (!strcmp(verb, "start")) {
		tim4_start();
	}

	if (!strcmp(verb, "test")) {
		tim4_test();
	}
*/
	tim4_usage();

	return -1;

}

