#include "pid.h"

#include <math.h>
#include <float.h>
#include <stdio.h>

#include "feedback.h"
#include "moving_avg.h"
#include "defines.h"
#include "cnc_param.h"
#include "cnc_func.h"

static BOOL i_ena, acc_req;
//static float gainI = PID_GAIN_I;

static float F0 = 0, F = 0; // mm / clock
//static float I = 0;

void pid_clear() {
	i_ena = acc_req = FALSE;
	F0 = F = 0;
	mavg_clear();

	printf("F:0\n");
}

void pid_stop() {
	F0 = F = 0;
	if (i_ena) {
		acc_req = TRUE;
		printf("F:0\n");
	}
	else
		pid_clear();
}

void pid_reset() {
	pid_clear();
//	gainI = PID_GAIN_I;
}

int mavg_n();

float pid(uint16_t adc, float Tnom, float dL) {
	static float err, dF, Fp, Fi, Fmax, Tp, Ti, T;

	Fmax = 1.0f / Tnom;

	if (acc_req)
		dF = cnc_pidAcc() * (100 * COE_VOLT_TO_DCODE); // err = 100 * (1.638 / 100) / (4.096 / 1<<10) = 410
	else {
		err = adc - fb_highThld();
		dF = err < 0 ? cnc_pidDec() * err : cnc_pidAcc() * err;
	}

	Fp = velocity(F0, dF, dL);

	// soft shortcut or acceleration
	if (i_ena && (Fp < F_MIN || Fp > Fmax)) {
		i_ena = FALSE;
		mavg_clear();

		Fp = velocity(F, dF, dL); // recalculate
	}

	if (Fp > Fmax) Fp = Fmax;
	Fp = speed_range(Fp);
	Tp = 1.0f / Fp;

	Ti = mavg_get();

	if (acc_req && Tp < Ti)
		acc_req = FALSE;
	else if (i_ena || (mavg_ready( MS_TO_TICK(5000) ) && Ti < Tp)) {
		i_ena = TRUE;
		Fi = 1.0f / Ti;
		F = (Fi + Fp) / 2;

		if (F > Fmax) F = Fmax;
		F = speed_range(F);
		T = 1.0f / F;
	}
	else {
		F = Fp;
		T = Tp;
	}

	//
	if (!acc_req)
		mavg_add(T); // Tp

	F0 = 2 * Fp - F0; // F1

	// debug
	float FF = (FPGA_CLOCK * 1000.0) * F;
	float FFp = (FPGA_CLOCK * 1000.0) * Fp;
	float FFi = (FPGA_CLOCK * 1000.0) * 1.0f / Ti;

	printf("err:%d F:%d Fp:%d Fi:%d(%x) n:%d\n", (int)err, (int)FF, (int)FFp, (int)FFi, (int)i_ena, mavg_n());

	return T;
}
