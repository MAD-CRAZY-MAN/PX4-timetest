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

extern "C" __EXPORT int tim4_main(int argc, char *argv[]);

static void timer_init()
{
	//init timer
	*((volatile uint32_t *)PWMIN_TIMER_POWER_REG) |= PWMIN_TIMER_POWER_BIT; //tim4 rcc enable  
	
	//1. configure the output pin
	rCCMR1 |= (0<<9 | 0 << 8); //CC2S output
	
	//2. select the pwm mode - mode 1
	rCCMR1 |= (1<<14 | 1<<13);
	if (rCCMR1 & (1<<12))
		rCCMR1 ^= (1<<12);	

	//3. program the period and the duty cycle
	rPSC |= 0b10000011001111; //prescaler: 8399
	rARR = 0x0000;
	rARR |= 0b10011100001111; //counter period: 9999(1s)	
	rCCR2 |= 0b1001110000111; //duty 50% output comapare mode
	
	//4. set the preload bit in CCMRx and the ARPE bit in the CR1
	rCCMR1 |= (1<<11); //output compare2 perload enable
	rCR1 |= (1 | (1<<7)); //counter enable, Auto-reload perload enable


	//5. slect the counting mode, DIR: upcounting
	rCR1 &= 0b1111111110001111; 

	exit(0);
	

}
static void timer_start()
{
	//1. enable the capture compare
	rCCER |= (1<<4);	
	//2. enable the counter
	rCR1 |= 1;
	       
	exit(0);
}

static void timer_end()
{
	//disable the capture compare
	if (rCCER & (1<<4))
		rCCER ^= (1<<4);
	//disable the counter
	if (rCR1 & 1)
		rCR1 ^= 1;
	exit(0);
}

static void timer_info(void)
{
	PX4_INFO("CLOCK and TIMER SETTING\n");
	printf("TIMER(APB1) CLOCK: %lu\n", PWMIN_TIMER_CLOCK);
	printf("PWMIN_TIMER: %d\n", PWMIN_TIMER);
	printf("PWM_TIMER_CHANNEL: %d\n\n", PWMIN_TIMER_CHANNEL);
	
	printf("rCR1: %x\n", rCR1);
	printf("rCR2: %x\n", rCR2);
	printf("rSMCR: %x\n", rSMCR);
	printf("rDIER: %x\n", rDIER);
	printf("rSR: %x\n", rSR);
	printf("rEGR: %x\n", rEGR);
	printf("rCCMR1: %x\n", rCCMR1);
	printf("rCCMR2: %x\n", rCCMR2);
	printf("rCCER: %x\n", rCCER);
	printf("rCNT: %x\n", rCNT);
	printf("prescaler: %d\n", rPSC);
	printf("ARR: %d\n", rARR);
	printf("rCCR1: %x\n", rCCR1);
	printf("rCCR2: %x\n", rCCR2);
	printf("rCCR3: %x\n", rCCR3);
	printf("rCCR4: %x\n", rCCR4);
	printf("rDCR: %x\n", rDCR);
	printf("rDMAR: %x\n", rDMAR);
	exit(0);
}

static void tim4_usage()
{
	PX4_ERR("unrecognized command, try 'info', 'init' and 'start' or 'end'");
}

int tim4_main(int argc, char *argv[])
{
	if(argc <2){
		tim4_usage();
		return -1;
	}

	const char *verb = argv[1];

	if (!strcmp(verb, "info")) {
		timer_info();
	}
	
	if (!strcmp(verb, "init")){
		timer_init();
	}
	
	if (!strcmp(verb, "start")) {
		timer_start();
	}

	if (!strcmp(verb, "end")) {
		timer_end();
	}

	tim4_usage();

	return -1;

}

