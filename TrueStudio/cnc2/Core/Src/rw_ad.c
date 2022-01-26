#include <stdio.h>

#include "stm32h7xx_hal.h"

#include "defines.h"
#include "rw_ad.h"
#include "aux_func.h"
#include "prog_array.h"
#include "cnc_task.h"
#include "fpga.h"
#include "fpga_gpo.h"
#include "tx_buf.h"
#include "imit_fifo.h"
#include "step_dir.h"
#include "debug_fifo.h"
#include "backup.h"
#include "context.h"
#include "feedback.h"
#include "encoder.h"
#include "pid.h"
#include "center.h"
#include "cnc_func.h"

static uint32_t test_reg = 0x12345678;
static center_t center = {1, 0, 0, CENTER_NO, 100.0f, 0.0f, 300.f, 30.0f};

void reset() {
	cnc_reset();
	test_reg = 0x12345678;
	imit_fifo_clear();
	debug_fifo_clear();
}

void ad_writeRegs(size_t addr, size_t len, const uint8_t buf[], size_t N) {
	uint32_t addr32;
	size_t len32;
	size_t pos = 5;

	size_t len_max = N - 9;
	if (len > len_max) len = len_max;

	if ((addr & ADDR_MASK) == PA_BAR) {
		pa_writeBytes(addr & ~ADDR_MASK, len, buf, N, pos);
		tx_wrack(addr, len);
	}
	else if ((addr & ADDR_MASK) == MCU_BAR && (addr & 3) == 0 && (len & 3) == 0) {
		addr32 = (addr & ~ADDR_MASK) >> 2;
		len32 = len >> 2;

		for (int i = 0; i < len32; i++, addr32++, pos += sizeof(uint32_t)) {
			uint32_t wrdata = read_u32(buf, N, pos);

			const uint8_t* const bytes = (uint8_t*)&wrdata;
			const int32_t* const p32 = (int32_t*)&wrdata;
			const float* const pfloat = (float*)&wrdata;

			if (addr32 < 0x200)
				switch (addr32) {
					case 0:
	//					printf("W: A0 %08x\n", wrdata);
						if (wrdata & 1) cnc_runReq();

						if (wrdata & 1<<8) cnc_revReq();

						if (wrdata & 1<<16) step_setImitEna(TRUE);
						break;
					case 1:
						if (wrdata & 1<<0) cnc_stopReq();
						if (wrdata & 1<<1) cnc_abortReq();

						if (wrdata & 1<<3) cnc_reset();

						if (wrdata & 1<<16) step_setImitEna(FALSE);
						break;

					case 0x10: cnc_enableUV(wrdata != 0); break;

					case 0x12: cnc_goto(wrdata); break;
					case 0x13: cnc_setParam(0, *p32); break; // x
					case 0x14: cnc_setParam(1, *p32); break; // y
					case 0x15: cnc_setParam(2, *p32); break; // u
					case 0x16: cnc_setParam(3, *p32); break; // v
					case 0x17: cnc_setParam(4, *p32); break; // enc_x
					case 0x18: cnc_setParam(5, *p32); // enc_y
						cnc_recovery();
						break;

					case 0x20: gpo_setControlsEnable((uint16_t)wrdata, (uint16_t)(wrdata>>16)); break;
					case 0x21: gpo_setDrumVel(wrdata); break;
					case 0x22: gpo_setVoltageLevel(wrdata); break;
					case 0x23: gpo_setCurrentIndex(wrdata); break;
					case 0x24: gpo_setPulseWidth(wrdata); break;
					case 0x25: gpo_setPulseRatio(wrdata); break;
					case 0x26: cnc_setSpeed(*pfloat); break;
					case 0x27: /*cnc_setStep(*pfloat);*/ break;
					// TODO: enc_ena

					// Settings
					case 0x30:
						cnc_reset();
						fpga_setInputLevel((uint16_t)wrdata);
						printf("WR inLvl:%x\n", (unsigned)wrdata);
						break;
					case 0x31:
						fpga_setMotorDir((uint16_t)wrdata);
						break;
					case 0x32:
						cnc_setAcc(*pfloat); // um/sec2
						break;
					case 0x33:
						cnc_setDec(*pfloat); // um/sec2
						break;

					case 0x38:
						fb_enable(wrdata & 1);
						printf("FB_ENA:%x\n", (int)fb_isEnabledForce());
						break;
					case 0x39:
						fb_setThld((uint16_t)wrdata, (uint16_t)(wrdata>>16)); // low, high
						break;
					case 0x3A:
						fb_setRollbackTimeout(wrdata);
						break;
					case 0x3B:
						cnc_setRollbackAttempts(wrdata);
						break;
					case 0x3C:
						cnc_setRollbackLength(*pfloat); // mm
						break;
					case 0x3D:
						fb_setRollbackSpeed(*pfloat); // mm/min
						break;

					case 0x40: cnc_setParam(0, *p32); break;
					case 0x41: cnc_setParam(1, *p32); break;
					case 0x42: cnc_setParam(2, *p32); break;
					case 0x43: cnc_setParam(3, *p32); break;
					case 0x44: cnc_setParam(4, *p32); break;
					case 0x45:
						cnc_setParam(5, *p32);

						if (cnc_isInit()) {
							cnc_reqG92();
						}
						break;

					case 0x50: cnc_setParam(0, *p32); break;
					case 0x51: cnc_setParam(1, *p32); break;
					case 0x52: cnc_setParam(2, *p32); break;
					case 0x53: cnc_setParam(3, *p32); break;
					case 0x54: cnc_setParam(4, *p32); break;
					case 0x55: cnc_setParam(5, *p32); break;
					case 0x56: cnc_setParam(6, *p32); break;
					case 0x57:
						cnc_setParam(7, *p32);

						if (cnc_isInit()) {
							cnc_reqG1();
						}
						break;

					case 0x70: center.mode		= (CENTER_MODE_T)bytes[0];
							   center.attempts	= bytes[1];
							   center.fineZonePct	= bytes[2];
							   center.drumVel	= bytes[3];
						break;
					case 0x71: center.R			= *pfloat; break;
					case 0x72: center.rollback	= *pfloat; break;
					case 0x73: center.F			= mmm_to_mmclock(*pfloat); break;
					case 0x74: center.F_fine	= mmm_to_mmclock(*pfloat);
						cnc_reqCenter(&center);
						break;

					case 0xFF: test_reg = wrdata; break;

	//				case 0x100: if (!cnc_run()) pa_setBegin(wrdata); break;
					case 0x101: pa_setWraddr(wrdata); break;
	//				case 0x102: if (!cnc_run()) pa_setRev(wrdata); break;
					default: break;
				}
			else if (addr32 < 0x220)
				bkp_writeRegU32(addr32 & 0x1F, wrdata);
		}

		if (!gpo_getValid())
			gpo_apply();

		tx_wrack(addr, len);
	}
	else if ((addr & ADDR_MASK) == FPGA_BAR && (addr & 1) == 0 && (len & 1) == 0) {
		uint32_t addr16 = (addr & ~ADDR_MASK) >> 1;
		size_t len16 = len >> 1;
		pos = 0;

		for (int i = 0; i < len16; i++, addr16++, pos += sizeof(uint16_t)) {
			uint16_t wrdata16 = read_u16(buf, N, pos);
			fpga_write_u16(addr16, wrdata16);
		}

		tx_wrack(addr, len);
	}
}

const char __date__[16] = __DATE__;
const char __time__[16] = __TIME__;

const uint32_t* __date32__ = (uint32_t*)__date__;
const uint32_t* __time32__ = (uint32_t*)__time__;

void ad_readRegs(uint32_t addr, size_t len) {
	static uint32_t rddata;
	static int32_t* const p32 = (int32_t*)&rddata;
	static float* const pfloat = (float*)&rddata;

	size_t pos = 5;

	if (len > sizeof(tx_buf) - 9) len = sizeof(tx_buf) - 9;

	if ((addr & ADDR_MASK) == PA_BAR) {
		pa_readBytes(addr & ~ADDR_MASK, len, tx_buf, sizeof(tx_buf), pos);
	}
	else if ((addr & ADDR_MASK) == MCU_BAR && (addr & 3) == 0 && (len & 3) == 0) {
		uint32_t addr32 = (addr & ~ADDR_MASK) >> 2;
		size_t len32 = len >> 2;

		for (int i = 0; i < len32; i++, addr32++, pos += sizeof(uint32_t)) {
			if (addr32 < 0x200)
				switch (addr32) {
					case 0: rddata = step_getImitEna()<<16 | cnc_isReverse()<<8 | cnc_error()<<3 | cnc_pause()<<2 | cnc_stop()<<1 | cnc_run(); break;

					case 0x10:
						cnc_ctx_getForce();
						rddata = cnc_ctx_get(0);
						break;
					case 0x11:
						rddata = cnc_ctx_get(1);
						break;
					case 0x12: // id
						rddata = cnc_ctx_get(2);
						break;
					case 0x13: // x
						rddata = cnc_ctx_get(3);
						break;
					case 0x14: // y
						rddata = cnc_ctx_get(4);
						break;
					case 0x15: // u
						rddata = cnc_ctx_get(5);
						break;
					case 0x16: // v
						rddata = cnc_ctx_get(6);
						break;
					case 0x17: // enc_x
						rddata = cnc_ctx_get(7);
						break;
					case 0x18: // enc_y
						rddata = cnc_ctx_get(8);
						break;
					case 0x19: // T
						rddata = cnc_ctx_get(9);
						break;
					case 0x1A: // T_cur
						rddata = cnc_ctx_get(10);
						break;
					case 0x1B: // step
						rddata = cnc_ctx_get(11);
						break;
					case 0x1C: // limsw, semaphore, feedback
						rddata = cnc_ctx_get(12);
						break;

					case 0x26: *pfloat = cnc_speed(); break;
					case 0x27: *pfloat = (float)cnc_step(); break;
					case 0x28: *pfloat = (float)SCALE; break;
					case 0x29: *pfloat = (float)SCALE; break;
					case 0x2A: *pfloat = (float)SCALE_UV; break;
					case 0x2B: *pfloat = (float)SCALE_UV; break;
					case 0x2C: *pfloat = (float)SCALE_ENC; break;
					case 0x2D: *pfloat = (float)SCALE_ENC; break;
					case 0x2E: rddata = 0; break;

					// Settings
					case 0x30:
						rddata = fpga_getInputLevel();
						printf("RD inLvl:%x\n", (unsigned)rddata);
						break;
					case 0x31:
						rddata = fpga_getMotorDir();
						break;
					case 0x32:
						*pfloat = cnc_acc(); // um/sec2
						break;
					case 0x33:
						*pfloat = cnc_dec(); // um/sec2
						break;

					case 0x38:
						rddata = (uint32_t)(fb_isEnabled() != 0);
						break;
					case 0x39:
						rddata = (uint32_t)fb_highThld()<<16 | (uint32_t)fb_lowThld(); // ADC bits
//						printf("RD THLD:%x %x\n", (unsigned)fpga_getHighThld(),.. (unsigned)fpga_getLowThld());
						break;
					case 0x3A:
						rddata = fb_getRollbackTimeout(); // ms
						break;
					case 0x3B:
						rddata = cnc_getRollbackAttempts();
						break;
					case 0x3C:
						*pfloat = cnc_getRollbackLength(); // mm
						break;
					case 0x3D:
						*pfloat = fb_getRollbackSpeed(); // mm/min
						break;

					case 0x40: *p32 = cnc_getParam(0); break;
					case 0x41: *p32 = cnc_getParam(1); break;
					case 0x42: *p32 = cnc_getParam(2); break;
					case 0x43: *p32 = cnc_getParam(3); break;
					case 0x44: *p32 = cnc_getParam(4); break;
					case 0x45: *p32 = cnc_getParam(5); break;

					case 0x50: *p32 = cnc_getParam(0); break;
					case 0x51: *p32 = cnc_getParam(1); break;
					case 0x52: *p32 = cnc_getParam(2); break;
					case 0x53: *p32 = cnc_getParam(3); break;
					case 0x54: *p32 = cnc_getParam(4); break;
					case 0x55: *p32 = cnc_getParam(5); break;
					case 0x56: *p32 = cnc_getParam(6); break;
					case 0x57: *p32 = cnc_getParam(7); break;

					case 0x60:
						fpga_adcSnapshop();
						rddata = (uint32_t)fpga_getADC(2)<<16 |  fpga_getADC(0); // back diff
						break;
					case 0x61:
						rddata = (uint32_t)fpga_getADC(5)<<16 | fpga_getADC(4); // wire- workpiece+
						break;
					case 0x62:
						rddata = (uint32_t)fpga_getADC(7)<<16 | fpga_getADC(6); // shunt hv+
						break;

					case 0x70: rddata = (uint32_t)center.drumVel<<24 | (uint32_t)center.fineZonePct<<16 | (uint32_t)center.attempts<<8 | center.mode; break;
					case 0x71: *pfloat = center.R; break;
					case 0x72: *pfloat = center.rollback; break;
					case 0x73: *pfloat = mmclock_to_mmm(center.F); break;
					case 0x74: *pfloat = mmclock_to_mmm(center.F_fine); break;
					case 0x75: *pfloat = center_D(AXIS_X); break;
					case 0x76: *pfloat = center_D(AXIS_Y); break;

					case 0xF0: rddata = __date32__[0]; break;
					case 0xF1: rddata = __date32__[1]; break;
					case 0xF2: rddata = __date32__[2]; break;
					case 0xF3: rddata = __date32__[3]; break;
					case 0xF4: rddata = __time32__[0]; break;
					case 0xF5: rddata = __time32__[1]; break;
					case 0xF6: rddata = __time32__[2]; break;
					case 0xF7: rddata = __time32__[3]; break;
					case 0xF8: rddata = SystemCoreClock; break;
					case 0xF9: rddata = VER_TYPE << 30 | FAC_VER << 24 | FAC_REV << 16 | VER << 8 | IS_STONE<<7 | REV; break;

					case 0xFF: rddata = test_reg; break;

					case 0x100: rddata = pa_getPos(); break;
					case 0x101: rddata = pa_getWraddr(); break;
					case 0x102: rddata = PA_SIZE; break;
					case 0x103: rddata = imit_fifo_count(); break;
					case 0x104: rddata = (uint32_t)cnc_getState(); break;
					case 0x105: *p32 = pa_getStrNum(); break;

					case 0x110:
					{
						motor_t* const m = imit_fifo_q();
						m->valid = !imit_fifo_empty();
						const uint32_t* const ptr32 = (uint32_t*)m;
						rddata = ptr32[0];
					}
						break;
					case 0x111:
					{
						const motor_t* const m = imit_fifo_q();
						const uint32_t* const ptr32 = (const uint32_t*)m;
						rddata = ptr32[1];
					}
						break;
					case 0x112:
					{
						const motor_t* const m = imit_fifo_q();
						const uint32_t* const ptr32 = (const uint32_t*)m;
						rddata = ptr32[2];
						imit_fifo_rdack();
					}
						break;

					default: rddata = 0; break;
				}
			else if (addr32 < 0x220)
				rddata = bkp_readRegU32(addr32 & 0x1F);
			else
				rddata = 0;

			write_u32(tx_buf, sizeof(tx_buf), pos, rddata);
		}
	}
	else if ((addr & ADDR_MASK) == FPGA_BAR && (addr & 1) == 0 && (len & 1) == 0) {
		uint32_t addr16 = (addr & ~ADDR_MASK) >> 1;
		size_t len16 = len >> 1;

		for (int i = 0; i < len16; i++, addr16++, pos += sizeof(uint16_t)) {
			uint16_t rddata16 = fpga_read_u16(addr16);
			write_u16(tx_buf, sizeof(tx_buf), pos, rddata16);
		}
	}
	else
		memset(&tx_buf[pos], 0, len);

	tx_readRegsAck(addr, len);
}

void ad_readFifo(uint32_t addr, size_t len) {
	uint32_t rddata;
	if (len > sizeof(tx_buf) - 9) len = sizeof(tx_buf) - 9;

	size_t pos = 5;

	if ((addr & ADDR_MASK) == MCU_BAR && (addr & 3) == 0 && (len & 3) == 0) {
		uint32_t addr32 = (addr & ~ADDR_MASK) >> 2;
		size_t len32 = len >> 2;

		if (addr32 == 0x110) {
			size_t new_len32 = 3 * imit_fifo_count();

			if (new_len32 > len32)
				len32 = (len32 / 3) * 3; // aligned number
			else
				len32 = new_len32;

			size_t cnt = 0;

			for (int i = 0; i < len32; i++, pos += sizeof(uint32_t)) {
				motor_t* const m = imit_fifo_q();
				m->valid = !imit_fifo_empty();
				const uint32_t* const ptr32 = (uint32_t*)m;

				if (cnt > 2) cnt = 0;
				rddata = ptr32[cnt++];

				if (cnt == 3 && m->valid) imit_fifo_rdack();

				write_u32(tx_buf, sizeof(tx_buf), pos, rddata);
			}

			tx_readFifoAck(addr, len32<<2);
			return;
		}
	}

	memset(&tx_buf[pos], 0, len);
	tx_readFifoAck(addr, len);
}
