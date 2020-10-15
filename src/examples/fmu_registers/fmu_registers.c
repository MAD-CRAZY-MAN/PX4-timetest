#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



#include <nuttx/arch.h>
#include <arch/board/board.h>

/* XXX might be able to prune these */
#include <chip.h>
#include <up_internal.h>
#include <up_arch.h>
#include <stm32.h>
#include <perf/perf_counter.h>

//#define DEBUG
//
#include "../../include/px4.h"
//static perf_counter_t	pc_txns;
//static perf_counter_t	pc_errors;
//static perf_counter_t	pc_ore;
//static perf_counter_t	pc_fe;
//static perf_counter_t	pc_ne;
//static perf_counter_t	pc_idle;
//static perf_counter_t	pc_badidle;
//static perf_counter_t	pc_regerr;
//static perf_counter_t	pc_crcerr;
//
//static void		rx_handle_packet(void);
//static void		rx_dma_callback(DMA_HANDLE handle, uint8_t status, void *arg);
//static DMA_HANDLE	tx_dma;
//static DMA_HANDLE	rx_dma;
//
//static int		serial_interrupt(int irq, void *context, FAR void *arg);
//static void		dma_reset(void);
//
//static struct IOPacket	dma_packet;
//
///* serial register accessors */

#define REG(_x)		(*(volatile uint32_t *)(STM32_USART2_BASE + _x))
#define rSR		REG(STM32_USART_SR_OFFSET)
#define rDR		REG(STM32_USART_DR_OFFSET)
#define rBRR		REG(STM32_USART_BRR_OFFSET)
#define rCR1		REG(STM32_USART_CR1_OFFSET)
#define rCR2		REG(STM32_USART_CR2_OFFSET)
#define rCR3		REG(STM32_USART_CR3_OFFSET)
#define rGTPR		REG(STM32_USART_GTPR_OFFSET)

__EXPORT int fmu_registers_main(int argc, char *argv[]);

int fmu_registers_main(int argc, char *argv[])
{
	PX4_INFO("USART2(TELEM1) REGISTERS\n");
	printf("rSR: %p, %p\n", rSR, &rSR);
	printf("rDR: %p, %p\n", rDR, &rDR);
	printf("rBRR: %p, %p\n", rBRR, &rBRR);
	printf("rCR1: %p, %p\n", rCR1, &rCR1);
	printf("rCR2: %p, %p\n", rCR2, &rCR2);
	printf("rCR3: %p, %p\n", rCR3, &rCR3);
	printf("rGTPR: %p, %p\n\n", rGTPR, &rGTPR);
	return OK;
}
