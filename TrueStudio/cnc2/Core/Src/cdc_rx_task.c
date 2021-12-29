#include <stdio.h>
#include "stm32h7xx_hal.h"

#include "my_types.h"
#include "rx_buf.h"

#define CDC_TIMEOUT (1000) // 1 second

static COMMAND_T getCommand() { return (COMMAND_T)(rx_fifo_get(0)>>4); }
static uint8_t getLength() { return rx_fifo_get(4); }

void cdc_rx_task() {
	static uint32_t tic;
	static size_t size_reg;
	static BOOL timer_ena;

	BOOL timer_req = FALSE;
	size_t size = rx_fifo_size();

	if (rx_buf_empty() && size) {
		timer_req = size == size_reg;

		if (size >= 9) {
			COMMAND_T cmd = getCommand();
			size_t len = getLength();

			switch (cmd) {
			case CMD_WRITE:
				len += 9;
				if (size >= len) {
					timer_req = FALSE;
					if (rx_buf_move(len) == 0)
						rx_fifo_flush();
				}
				break;

			case CMD_READ: case CMD_READ_FIFO:
				if (size >= 9) {
					timer_req = FALSE;
					if (rx_buf_move(9) == 0)
						rx_fifo_flush();
				}
				break;

			default:
				rx_fifo_flush();
				break;
			}
		}
	}

	if (timer_req) {
		if (!timer_ena) {
			timer_ena = TRUE;
			tic = HAL_GetTick();
		}
		else {
			uint32_t toc = HAL_GetTick();

			if (toc - tic >= CDC_TIMEOUT) {
				printf("RX FLUSH %d:\n", size);

				for (int i = 0; i < size; i++) {
					uint8_t c = rx_fifo_get(i);
					printf("%02x(%c) ", c, c);
				}

				printf("\n");
				rx_fifo_flush();
			}
		}
	}
	else
		timer_ena = FALSE;

	size_reg = size;
}
