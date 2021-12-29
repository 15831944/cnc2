#include <stdio.h>

#include "my_types.h"
#include "cdc_rx_task.h"
#include "aux_func.h"
#include "rx_buf.h"
#include "tx_buf.h"
#include "rw_ad.h"
#include "soft_wdt.h"

static void __error(COMMAND_T cmd, uint32_t addr) {
	tx_error(cmd, addr);
	rx_buf_rdack();
	rx_fifo_flush();
}

void com_task() {
	size_t size = rx_buf_size();

	if (size) {
		soft_wdt_clear();

		COMMAND_T cmd = rx_buf_cmd();
		uint32_t addr = rx_buf_addr();
		size_t len = rx_buf_len();
		const uint8_t* const bytes = (uint8_t*)rx_buf.array;

		switch (cmd) {
		case CMD_WRITE:
			if (size == len + 9) {
				uint32_t calc_crc = crc32(bytes, len + 5);
				uint32_t rx_crc = read_u32_rev(bytes, size, len + 5);

				if (calc_crc == rx_crc) {
					ad_writeRegs(addr, len, bytes, sizeof(rx_buf.array));
					rx_buf_rdack();
				}
				else {
//					printf("A%x L%d:\n", (int)addr, (int)len);
//					printf("CRC ERR %x (exp %x)\n", (int)rx_crc, (int)calc_crc);
					__error(cmd, addr);
				}
			}
			else
				__error(cmd, addr);

			break;

		case CMD_READ: case CMD_READ_FIFO:
			if (size == 9) {
				uint32_t calc_crc = crc32(bytes, 5);
				uint32_t rx_crc = read_u32_rev(bytes, size, 5);

				if (calc_crc == rx_crc) {
					cmd == CMD_READ ? ad_readRegs(addr, len) : ad_readFifo(addr, len);
					rx_buf_rdack();
				}
				else {
//					printf("C%d A%x L%d\n", rdevent.cmd, (int)rdevent.addr, (int)rdevent.len);
//					printf("CRC ERR %x (exp %x)\n", (int)rx_crc, (int)calc_crc);
					__error(cmd, addr);
				}
			}
			else
				__error(cmd, addr);

			break;

		default:
			rx_buf_rdack();
			break;
		}
	}
}
