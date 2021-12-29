#include "sem_led.h"
#include "fpga.h"
#include "cnc_task.h"

static uint8_t sem;
static BOOL sem_ena;

void sem_reset() { sem = 0; sem_ena = FALSE; }
SEM_COLOR_ENU sem_get() { return fpga_readSemReg() & SEM_MSK; }
BOOL sem_enabled() { return (fpga_readSemReg() & SEM_ENA_MSK) != 0; }

void sem_set(SEM_COLOR_ENU value) {
	sem = value;
	fpga_writeSemReg((uint16_t)sem_ena<<SEM_ENA_BIT | sem);
}

void sem_enable(BOOL value) {
	sem_ena = value;
	fpga_writeSemReg((uint16_t)sem_ena<<SEM_ENA_BIT | sem);
}

void sem_next() {
	switch (sem) {
	case SEM_OFF: sem_set(SEM_RED); break;
	case SEM_RED: sem_set(SEM_YELLOW); break;
	case SEM_YELLOW: sem_set(SEM_GREEN); break;
	case SEM_GREEN: sem_set(SEM_OFF); break;
	default: sem_set(SEM_OFF); break;
	}
}

void sem_task() {
	if (cnc_fault() || cnc_error())
		sem_set(SEM_RED);
	else if (cnc_stop())
		sem_set(SEM_YELLOW);
	else if (cnc_run())
		sem_set(SEM_GREEN);
	else
		sem_set(SEM_YELLOW);
}
