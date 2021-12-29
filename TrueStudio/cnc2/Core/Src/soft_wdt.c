#include "soft_wdt.h"

#include "stm32h7xx_hal.h"
#include "cnc_task.h"

static uint32_t tic, toc, cnt;
static BOOL wdt, enable;

void soft_wdt_reset() {
	tic = toc = cnt = 0;
	wdt = enable = FALSE;
}

BOOL soft_wdt() { return wdt; }

void soft_wdt_task() {
	if (enable) {
		toc = HAL_GetTick();

		if (toc - tic >= PC_WDT_PERIOD) {
			tic += PC_WDT_PERIOD;

			if (cnc_run() && !wdt && cnt) {
				cnc_onLoseConnection();
				wdt = TRUE;
			}

			cnt = 1;
		}
	}
	else {
		cnt = 0;
		wdt = FALSE;
	}
}

void soft_wdt_clear() {
	tic = HAL_GetTick();
	cnt = 0;
	wdt = FALSE;
}

void soft_wdt_enable(BOOL value) {
	if (value && enable) // already enabled
		return;

	soft_wdt_clear();
	enable = value;
}

BOOL soft_wdt_isEnabled() { return enable; }
