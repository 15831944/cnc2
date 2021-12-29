#include <stdio.h>

#include "flash_prog_array.h"

#include "quadspi.h"
#include "qspi_flash.h"

#include "my_types.h"
#include "my_lib.h"
#include "aux_func.h"
#include "cnc_func.h"

#define FLASH_PA_SIZE (MX25L512_PAGE_SIZE) // bytes
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

static flash_prog_array_t pa = {{0}, FALSE, FALSE, 0, 0, 0, PLANE_UNKNOWN};

size_t flash_pa_cap() { return FLASH_PA_SIZE; }

#define ERASE_MASK ((uint32_t)(MX25L512_SUBSECTOR_SIZE - 1))

/* Add string in tail of program array
*/
size_t flash_pa_write(const char* string) {
	uint8_t res;

	if (pa.flash_init) {
		size_t len = strlen(string) + 1;
		len = MIN(PA_BUF_SIZE, len);

		memcpy(pa.buf, string, len);
		pa.buf[PA_BUF_SIZE-1] = 0;
		pa.buf_valid = FALSE;

		if (pa.wraddr + len < FLASH_PA_SIZE) {
			uint32_t last_addr = pa.wraddr + len - 1;

			if ((pa.wraddr & ERASE_MASK) == 0 || (pa.wraddr & ~ERASE_MASK) != (last_addr & ~ERASE_MASK)) {
				printf("Erase %d... ", pa.wraddr);
				res = flash_pa_erase(pa.wraddr);
				QSPI_printResultValue(res);
			}

			res = BSP_QSPI_Write(pa.buf, pa.wraddr, len);

			if (res == QSPI_OK) {
				printf("W:%x: L%d\n", pa.wraddr, len);
				pa.wraddr += len;
				return len;
			}
			else {
				printf("W:");
				QSPI_printResultValue(res);
			}
		}
	}

	return 0;
}

void flash_pa_clear() {
	pa.buf_valid = FALSE;
	pa.wraddr = 0;
	pa.rdaddr = 0;
	pa.N = 0;
	pa.plane = PLANE_UNKNOWN;
}

BOOL flash_pa_init() {
	flash_pa_clear();
	uint8_t res = BSP_QSPI_Init();
	return pa.flash_init = res == QSPI_OK ? TRUE : FALSE;
}

void flash_pa_gotoBegin() {
	pa.buf_valid = FALSE;
	pa.rdaddr = 0;
	pa.N = 0;
	pa.plane = PLANE_UNKNOWN;
}

int flash_pa_count() {
	return pa.wraddr - pa.rdaddr;
}

int flash_pa_getWraddr() { return pa.wraddr; }
int flash_pa_getRdaddr() { return pa.rdaddr; }
int flash_pa_getStrNum() { return pa.N; }
PLANE_T flash_pa_plane() { return pa.plane; }

BOOL flash_pa_setWraddr(int value) {
//	if (value < FLASH_PA_SIZE && value >= 0 && value >= pa.rdaddr) {
	if (value >= 0 && value < FLASH_PA_SIZE) {
		pa.buf_valid = FALSE;
		pa.wraddr = value;

		if (pa.rdaddr > pa.wraddr)
			pa.rdaddr = pa.wraddr;

		pa.N = 0;
		pa.plane = PLANE_UNKNOWN;

		return TRUE;
	}

	return FALSE;
}

const char* flash_pa_current() {
	if (!pa.flash_init) return NULL;

	int count = flash_pa_count();

	if (count <= 0)
		return NULL;

	if (count == 1) {
		pa.buf[0] = '\0';
		return (char*)pa.buf;
	}

	uint8_t res = BSP_QSPI_Read(pa.buf, pa.rdaddr, MIN(PA_BUF_SIZE, count));

	if (res != QSPI_OK) return NULL;

	if (count < PA_BUF_SIZE)
		memset(&pa.buf[count], 0, PA_BUF_SIZE - count);
	else
		pa.buf[PA_BUF_SIZE - 1] = '\0';

	pa.buf_valid = TRUE;
	return (char*)pa.buf;
}

uint8_t* flash_pa_current_rev() {
	if (pa.flash_init) {
		uint32_t size = pa.rdaddr > PA_BUF_SIZE ? PA_BUF_SIZE : pa.rdaddr;
		uint32_t addr = pa.rdaddr - size;

		uint8_t res = BSP_QSPI_Read(pa.buf + PA_BUF_SIZE - size, addr, size); // write to end of buffer

		if (res == QSPI_OK) {
			if (size < PA_BUF_SIZE)
				memset(pa.buf, 0, PA_BUF_SIZE - size);

			return pa.buf;
		}
	}

	return NULL;
}

BOOL flash_pa_next() {
	const char* str = flash_pa_current();

	if (str) {
		int len = flash_buf_strlen() + 1;

		if (len > 0) {
			pa.N++;
			pa.rdaddr += len;

			if (pa.rdaddr > pa.wraddr)
				pa.rdaddr = pa.wraddr;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL flash_pa_prev() {
	uint8_t* str = flash_pa_current_rev();

	if (str) {
		int len = flash_buf_strlen_rev() + 1;

		if (len > 0 && pa.N > 0) {
			pa.N--;
			pa.rdaddr -= len;
			return TRUE;
		}
	}

	return FALSE;
}

void flash_pa_goto(int str_num) {
	BOOL OK = FALSE;

	if (str_num < 0)
		str_num = 0;

	if (str_num != pa.N) {
		if (pa.N < str_num) {
			do {
				OK = flash_pa_next();
			} while (pa.N < str_num && OK);
		}
		else {
			do {
				OK = flash_pa_prev();
			} while (pa.N > 0 && pa.N > str_num && OK);
		}
	}
}

static int rdaddr_reg;
static int N_reg;

static void __pa_store() {
	rdaddr_reg = pa.rdaddr;
	N_reg = pa.N;
}

static void __pa_restore() {
	pa.rdaddr = rdaddr_reg;
	pa.N = N_reg;
}

const char* flash_pa_try(int str_num) {
	const char* res;
	__pa_store();

	flash_pa_goto(str_num);
	res = flash_pa_current();

	__pa_restore();
	pa.buf_valid = FALSE;
	return res;
}

int flash_buf_strlen() {
	int i;

	if (pa.buf_valid) {
		for (i = 0; pa.buf[i] != '\0' && i < PA_BUF_SIZE; i++);

		if (i >= PA_BUF_SIZE)
			return -1;

		return i;
	}

	return -1;
}

// Address must pointed to index after last string
int flash_buf_strlen_rev() {
	int i;

	if (pa.buf_valid) {
		for (i = PA_BUF_SIZE - 1; pa.buf[i] != '\0' && i >= 0; i--);

		if (i >= -1)
			return PA_BUF_SIZE - 1 - i;
		else if (pa.buf[0] == '\0')
			return PA_BUF_SIZE;
		else
			return -1;
	}

	return -1;
}

void flash_pa_readBytes(uint32_t addr, size_t len, uint8_t buf[], size_t N, size_t pos) {
	if (pa.flash_init && addr < FLASH_PA_SIZE)
		if (addr + len <= FLASH_PA_SIZE) {
			uint8_t res = BSP_QSPI_Read(&buf[pos], addr, MIN(len, N - pos));

			if (res != QSPI_OK)
				memset(&buf[pos], 0xE0, MIN(len, N - pos));
		}
		else
			memset(&buf[pos], 0xE1, MIN(len, N - pos));
	else
		memset(&buf[pos], 0xE2, MIN(len, N - pos));
}

void flash_pa_writeBytes(uint32_t addr, size_t len, uint8_t buf[], size_t N, size_t pos) {
	len = pos + len <= N ? len : N - pos;

	if (pa.flash_init && addr + len <= FLASH_PA_SIZE)
		BSP_QSPI_Write(&buf[pos], addr, MIN(len, N - pos));
}

BOOL flash_pa_erase(uint32_t addr) {
	if (pa.flash_init) {
		uint8_t res = BSP_QSPI_Erase_Block(addr);
		if (res == QSPI_OK) return TRUE;
	}

	return FALSE;
}

void flash_pa_writeBytesStream(uint32_t addr, size_t len, uint8_t buf[], size_t N, size_t pos) {
	len = pos + len <= N ? len : N - pos;

	uint32_t last_addr = addr + len - 1;

	if ((addr & ERASE_MASK) == 0 || (addr & ~ERASE_MASK) != (last_addr & ~ERASE_MASK))
		flash_pa_erase(addr);

	flash_pa_writeBytes(addr, len, buf, N, pos);
}

//BOOL pa_getGCmd(const gcmd_t* cmd, int const* G) {
//	if (!pa.s || *pa.s == '\0' || *pa.s < &data[rdaddr] || *pa.s >= &data[wraddr])
//		pa.s = pa_current();
//
//	BOOL OK = gcode_parse(&pa.s, cmd, G);
//	gcmd_print(cmd);
//	return OK;
//}
//
//BOOL flash_pa_getGCmd(gcmd_t* const cmd, int const* G) {
//	BOOL OK = gcode_parse(flash_pa_current(), cmd, size, NULL); // todo
//	gcmd_print(cmd);
//	return OK;
//}
//
//// note: all frame must have segment type
//BOOL flash_pa_getSegment(gcmd_t* const cmd, gline_t* const gline, gline_t* const uv_gline, garc_t* const garc, garc_t* const uv_garc) {
//	static gcmd_t cmd_prev;
//	static fpoint_t A, A2, B, B2, C, C2;
//
//	BOOL OK = flash_pa_getGFrame(cmd);
//
//	gline_clear(gline);
//	gline_clear(uv_gline);
//	garc_clear(garc);
//	garc_clear(uv_garc);
//
//	if (pa.plane == PLANE_UNKNOWN) {
//		if (gcmd_isUV(cmd))
//			pa.plane = PLANE_XYUV;
//		else if (gcmd_isXY(cmd))
//			pa.plane = PLANE_XY;
//	}
//
//	BOOL uv_ena = pa.plane == PLANE_XYUV;
//
//	if (OK && cmd->valid.flags.G && cmd->G <= 3) {
//		struct {
//			uint8_t x:1;
//			uint8_t y:1;
//			uint8_t u:1;
//			uint8_t v:1;
//		} valid = {0,0,0,0};
//
//		A.x = 0;
//		A.y = 0;
//		A2.x = 0;
//		A2.y = 0;
//
//		__pa_store();
//
//		while (1) {
//			if (flash_pa_prev() == FALSE)
//				break;
//			else {
//				OK = flash_pa_getGFrame(&cmd_prev);
//
//				if (OK && cmd->valid.flags.G && (cmd->G <= 3 || cmd->G == 92)) {
//					if (!valid.x && cmd_prev.valid.flags.X) {
//						A.x = cmd_prev.X;
//						valid.x = 1;
//					}
//					if (!valid.y && cmd_prev.valid.flags.Y) {
//						A.y = cmd_prev.Y;
//						valid.y = 1;
//					}
//					if (!valid.u && cmd_prev.valid.flags.U) {
//						A2.x = cmd_prev.U;
//						valid.u = 1;
//					}
//					if (!valid.v && cmd_prev.valid.flags.V) {
//						A2.y = cmd_prev.V;
//						valid.v = 1;
//					}
//				}
//
//				if ((!uv_ena && valid.x && valid.y) || (uv_ena && valid.x && valid.y && valid.u && valid.v))
//					break;
//			}
//		}
//
//		__pa_restore();
//
//		B = A;
//		B2 = A2;
//
//		if (cmd->valid.flags.X)
//			B.x = cmd->X;
//
//		if (cmd->valid.flags.Y)
//			B.y = cmd->Y;
//
//		if (cmd->valid.flags.U)
//			B2.x = cmd->U;
//
//		if (cmd->valid.flags.V)
//			B2.y = cmd->V;
//
//		C = A;
//		C2 = A2;
//
//		if (cmd->valid.flags.I)
//			C.x += cmd->I;
//
//		if (cmd->valid.flags.J)
//			C.y += cmd->J;
//
//		if (cmd->valid.flags.I2)
//			C2.x += cmd->I2;
//
//		if (cmd->valid.flags.J2)
//			C2.y += cmd->J2;
//
//
//		switch (cmd->G) {
//		case 0: case 1:
//			gline->valid = TRUE;
//			gline->A = A;
//			gline->B = B;
//			break;
//
//		case 2: case 3:
//			garc->flag.valid = 1;
//			garc->flag.ccw = cmd->G == 3;
//			garc->flag.R = 0;
//			garc->A = A;
//			garc->B = B;
//			garc->C = C;
//			break;
//		}
//
//		if (uv_ena && cmd->valid.flags.G2 && cmd->G2 <= 3)
//			switch (cmd->G2) {
//			case 0: case 1:
//				uv_gline->valid = TRUE;
//				uv_gline->A = A2;
//				uv_gline->B = B2;
//				break;
//
//			case 2: case 3:
//				uv_garc->flag.valid = 1;
//				uv_garc->flag.ccw = cmd->G2 == 3;
//				uv_garc->flag.R = 0;
//				uv_garc->A = A2;
//				uv_garc->B = B2;
//				uv_garc->C = C2;
//				break;
//			}
//
//		return TRUE;
//	}
//
//	return OK;
//}

// Print program array as string array
void flash_pa_print() {
	const char* str = flash_pa_current();

	while (str) {
		for (int i = 0; i < PA_BUF_SIZE; i++, str++) {
			if (*str == '\0') {
				printf("\n");
				break;
			}
			else
				printf("%c", *str);
		}

		flash_pa_next();
		str = flash_pa_current();
	}

//	if (pa.data[pa.wraddr - 1] != '\0')
//		printf("\nwraddr ERROR!\n");
}

// Print current segment
//void flash_pa_printSegment() {
//	static gcmd_t cmd;
//	static gline_t gline, uv_gline;
//	static garc_t garc, uv_garc;
//
//	BOOL OK = flash_pa_getSegment(&cmd, &gline, &uv_gline, &garc, &uv_garc);
//
//	if (!OK) {
//		printf("Seg Error\n");
//		return;
//	}
//
//	if (gline.valid)
//		gline_print(&gline);
//	if (garc.flag.valid)
//		garc_print(&garc);
//	if (uv_gline.valid)
//		gline_print(&uv_gline);
//	if (uv_garc.flag.valid)
//		garc_print(&uv_garc);
//}

void flash_pa_test1() {
#ifdef FLASH_PA_TEST
	static char wrstr[64];
	const int base = 0;
	size_t cnt = 0;

	BOOL OK = flash_pa_init();
	OK ? printf("Flash Init OK\n") : printf("Flash Init ERR\n");
	if (!OK) return;

	for (int i = 0; i < 200; i++) {
		memset(wrstr, 0, sizeof(wrstr) - 1);
		snprintf(wrstr, sizeof(wrstr), "String %d", base + i);
		wrstr[sizeof(wrstr) - 1] = 0;

		printf("W:%s\n", wrstr);
		cnt += flash_pa_write(wrstr);
	}

	printf("Wrote %d bytes\n", cnt);

	const char* rdstr = flash_pa_current();

	while (rdstr) {
		printf("R:%s\n", rdstr);
		flash_pa_next();
		rdstr = flash_pa_current();
	}

	printf("Flash test 1 FIN\n");
#endif
}

void flash_pa_test2() {
#ifdef FLASH_PA_TEST
//	const char* test[] = {
//		"%",
//		"G92 X0 Y0",
//		"G1 Y-150",
//		"G3 X50 Y-200 I50",
//		"G1 X150",
//		"G3 X200 Y-150 J50",
//		"G1 Y-50",
//		"G3 X150 Y0 I-50",
//		"G1 X50",
//		"G3 X0 Y-50 J-50",
//		"M2",
//		"%"
//	};
//
//	pa_clear();
//	pa_setBegin(0);
//
//	for (int i = 0; i < sizeof(test)/sizeof(char*); i++)
//		pa_write(test[i]);
//
//	pa_print();
//
//	{
//		decimal_t x = float2fix(10.02);
//		printf("%d.%03d\n", x.value, x.rem);
//	}
//
//	{
//		line_t line;
//		line_init(&line, 10, 20, 30, 40, FALSE);
//		line_print(&line);
//	}
//
//	{
//		arc_t arc;
//		arc_initCenter(&arc, 10, 10, 30, 10, 20, 10, FALSE);
//		arc_print(&arc);
//
//		arc_initCenter(&arc, 10, 10, 30, 10, 20, 10, TRUE);
//		arc_print(&arc);
//
//		arc_initCenter(&arc, 10, 10, 20, 20, 20, 10, FALSE);
//		arc_print(&arc);
//
//		arc_initCenter(&arc, 10, 10, 20, 20, 20, 10, TRUE);
//		arc_print(&arc);
//	}
//
//	int N = 0;
//	while (pa_goto(N++) != NULL) {
//		pa_printSegment();
//	}
#endif
}

void flash_pa_test() {
#ifdef FLASH_PA_TEST
	switch (FLASH_PA_TEST) {
	default: flash_pa_test1(); break;
	case 2: flash_pa_test2(); break;
	}
#endif
}
