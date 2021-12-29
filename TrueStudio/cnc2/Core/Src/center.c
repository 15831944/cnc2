#include "center.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fpga.h"
#include "step_dir.h"
#include "cnc_param.h"
#include "feedback.h"
#include "defines.h"
#include "cnc_func.h"
#include "fpga_gpo.h"

static center_t center;
static enum {ST_IDLE, ST_FWD, ST_FINE, ST_FWD0, ST_FWD_FINE0, ST_REV_FINE0, ST_FWD1, ST_FWD_FINE1, ST_REV_FINE1, ST_CENTER, ST_WAIT} state, state_old;
static AXIS_T axis = AXIS_X;
static int32_t nx, ny, nx_slow, ny_slow;
static double ts = 0;
static int C_acc[2], pos[2];
static int acc_cnt = 1;
static point_t pt[2];
static int32_t D_acc[2], D[2];

void center_default() {
	center.mode			= CENTER_NO;
	center.attempts		= 1;
	center.fineZonePct	= 0;
	center.drumVel		= 0;
	center.R		= 100; // mm
	center.rollback	= 0; // mm
	center.F		= mmm_to_mmclock(18);
	center.F_fine	= mmm_to_mmclock(1.8);
}

void center_reset() {
	state_old = state = ST_IDLE;
	axis = AXIS_X;
	nx = ny = nx_slow = ny_slow = 0;
	ts = 0;
	C_acc[0] = C_acc[1] = 0;
	pos[0] = pos[1] = 0;
	acc_cnt = 1;
	pt[0].x = pt[0].y = 0;
	pt[1].x = pt[1].y = 0;
	D[0] = D[1] = 0;
	D_acc[0] = D_acc[1] = 0;
}

int32_t center_D(AXIS_T axis) { return axis < sizeof(D)/sizeof(D[0]) ? D[axis] : 0; }

// return 0 - OK
int center_init(const center_t* const p) {
	center_reset();

	memcpy(&center, p, sizeof(center_t));

	if (	(center.mode == CENTER_X || center.mode == CENTER_Y || center.mode == CENTER_CIRCLE) &&
			center.attempts && center.attempts < CENTER_ATTEMPTS_MAX &&
			center.fineZonePct >= 0 && center.fineZonePct <= 100 &&
			center.drumVel >= 0 && center.drumVel <= DRUM_VEL_MAX &&
			fabs(center.R) < CENTER_RADIUS_MAX && fabs(center.rollback) < CENTER_RADIUS_MAX &&
			center.F >= F_MIN && center.F <= F_MAX &&
			center.F_fine >= F_MIN && center.F_fine <= F_MAX
	){
		printf("CENTER Md=%d Att=%d Z=%d Dr=%d R=%d RB=%d V=%d VF=%d\n",
				(int)center.mode, (int)center.attempts, (int)center.fineZonePct, (int)center.drumVel,
				(int)(center.R * 1000), (int)(center.rollback * 1000),
				(int)(mmclock_to_mmm(center.F) * (1000.0/60)), (int)(mmclock_to_mmm(center.F_fine) * (1000.0/60))
			);

		return 0;
	}

	center_default();
	return -1;
}

void cnc_reqG92_xyuv_enc(int32_t x, int32_t y, int32_t u, int32_t v, int32_t enc_x, int32_t enc_y);

// return: 0 - OK, 1 - finish, result < 0 - error
int center_next(float* const T) {
	state_old = state;

	switch (state) {
	default:
		center_reset();
		center_default();
		return -1;

	case ST_IDLE:
		if (center.mode == CENTER_X || center.mode == CENTER_Y) {
			nx = center.mode == CENTER_X ? mm_to_steps(center.R, cnc_scale_x()) : 0;
			ny = center.mode == CENTER_Y ? mm_to_steps(center.R, cnc_scale_y()) : 0;

			if (nx || ny) {
				cnc_reqG92_xyuv_enc(0, 0, 0, 0, 0, 0);

				*T = 1.0f / center.F; // clocks / mm
				ts = fabs(center.R) * (*T); // clocks

				fpga_setCenterMode(FPGA_CENTER_FORWARD);
				gpo_setDrumVel(center.drumVel);
				gpo_setDrumEnable(center.drumVel != 0);
				step_writeXYUV(0, ts, nx, ny, 0, 0);
				state = ST_FWD;
			}

			printf("C:%c x:%d(%d) y:%d(%d)\n", center.mode == CENTER_X ? 'X' : 'Y', (int)nx, (int)(nx ? round(ts/nx) : 0), (int)ny, (int)(ny ? round(ts/ny) : 0));
		}
		else if (center.mode == CENTER_CIRCLE) {
			axis = AXIS_X;
			C_acc[0] = C_acc[1] = 0;
			D_acc[0] = D_acc[1] = 0;
			acc_cnt = 1;
			D[0] = D[1] = 0;

			cnc_reqG92_xyuv_enc(0, 0, 0, 0, 0, 0);

			float pct = (100 - center.fineZonePct) * 0.001;
			float R = center.R * pct;
			nx = mm_to_steps(R, cnc_scale_x());
			ny = 0;

			if (nx || ny) {
				*T = 1.0f / center.F; // clocks / mm
				ts = fabs(R) * (*T); // clocks

				fpga_setCenterMode(FPGA_CENTER_FORWARD);
				gpo_setDrumVel(center.drumVel);
				gpo_setDrumEnable(center.drumVel != 0);
				step_writeXYUV(0, ts, nx, ny, 0, 0);
				state = ST_FWD0;
			}

			printf("C:CIR x:%d(%d) y:%d(%d)\n", (int)nx, (int)(nx ? round(ts/nx) : 0), (int)ny, (int)(ny ? round(ts/ny) : 0));
		}
		break;

	case ST_FWD:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				nx = -fpga_getPos(0); // goto (0,0)
				ny = -fpga_getPos(1);

				if (nx || ny) {
					*T = 1.0f / center.F_fine; // clocks / mm
					ts = fabs(center.R) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_REVERSE);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("Rev x:%d(%d) y:%d(%d)\n", (int)nx, (int)(nx ? round(ts/nx) : 0), (int)ny, (int)(ny ? round(ts/ny) : 0));

					state = ST_FINE;
					break;
				}
			}
		}

		center_reset();
		center_default();
		return -1;

	case ST_FINE:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				nx = center.mode == CENTER_X ? mm_to_steps(center.rollback, cnc_scale_x()) : 0;
				ny = center.mode == CENTER_Y ? mm_to_steps(center.rollback, cnc_scale_y()) : 0;

				*T = 1.0f / center.F_fine; // clocks / mm
				ts = fabs(center.rollback) * (*T); // clocks

				fpga_setCenterMode(FPGA_CENTER_OFF);
				step_writeXYUV(0, ts, nx, ny, 0, 0);

				printf("RB x:%d y:%d\n", (int)nx, (int)ny);

				state = ST_WAIT;
				break;
			}
		}

		center_reset();
		center_default();
		return -1;

	case ST_FWD0:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				nx = -fpga_getPos(0); // goto (0,0)
				ny = -fpga_getPos(1);

				if (nx || ny) {
					*T = 1.0f / center.F_fine; // clocks / mm
					ts = steps_to_mm2(nx, ny, cnc_scale_xy()) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_REVERSE);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("Rev x:%d(%d) y:%d(%d)\n", (int)nx, (int)(nx ? round(ts/nx) : 0), (int)ny, (int)(ny ? round(ts/ny) : 0));

					state = ST_REV_FINE0;
					break;
				}
			}
			else {
				nx = axis == AXIS_X ? mm_to_steps(center.R * center.fineZonePct * 0.01, cnc_scale_x()) : 0;
				ny = axis == AXIS_Y ? mm_to_steps(center.R * center.fineZonePct * 0.01, cnc_scale_y()) : 0;

				if (nx || ny) {
					*T = 1.0f / center.F; // clocks / mm
					ts = fabs(center.R) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_FORWARD);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("Fwd x:%d(%d) y:%d(%d)\n", (int)nx, (int)(nx ? round(ts/nx) : 0), (int)ny, (int)(ny ? round(ts/ny) : 0));

					state = ST_FWD_FINE0;
					break;
				}
			}
		}

		center_reset();
		center_default();
		return -1;

	case ST_FWD_FINE0:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				nx = -fpga_getPos(0); // goto (0,0)
				ny = -fpga_getPos(1);

				if (nx || ny) {
					*T = 1.0f / center.F_fine; // clocks / mm
					ts = steps_to_mm2(nx, ny, cnc_scale_xy()) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_REVERSE);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("Rev %d %d\n", (int)nx, (int)ny);

					state = ST_REV_FINE0;
					break;
				}
			}
		}

		center_reset();
		center_default();
		return -1;

	case ST_REV_FINE0:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				pos[0] = fpga_getPos(axis);

				float double_pct = (100 - center.fineZonePct) * (-2 * 0.001);
				float R = center.R * double_pct;

				nx = axis == AXIS_X ? mm_to_steps(R, cnc_scale_x()) : 0;
				ny = axis == AXIS_Y ? mm_to_steps(R, cnc_scale_y()) : 0;

				if (nx || ny) {
					*T = 1.0f / center.F; // clocks / mm
					ts = steps_to_mm2(nx, ny, cnc_scale_xy()) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_OFF);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("RB %d %d\n", (int)nx, (int)ny);

					state = ST_FWD1;
					break;
				}
			}
		}

		center_reset();
		center_default();
		return -1;

	case ST_FWD1:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				nx = -fpga_getPos(0); // goto (0,0)
				ny = -fpga_getPos(1);

				if (nx || ny) {
					*T = 1.0f / center.F_fine; // clocks / mm
					ts = fabs(center.R) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_REVERSE);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("Move %d %d\n", (int)nx, (int)ny);

					state = ST_REV_FINE1;
					break;
				}
			}
			else {
				nx = axis == AXIS_X ? -mm_to_steps(center.R * center.fineZonePct * 0.01, cnc_scale_x()) : 0;
				ny = axis == AXIS_Y ? -mm_to_steps(center.R * center.fineZonePct * 0.01, cnc_scale_y()) : 0;

				if (nx || ny) {
					*T = 1.0f / center.F; // clocks / mm
					ts = fabs(center.R) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_FORWARD);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("Mode:Ñ Tx:%d Ty:%d x:%d y:%d\n", (int)(nx ? round(ts/nx) : 0), (int)(ny ? round(ts/ny) : 0), (int)nx, (int)ny);

					state = ST_FWD_FINE1;
					break;
				}
			}
		}

		center_reset();
		center_default();
		return -1;

	case ST_FWD_FINE1:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				nx = -fpga_getPos(0); // goto (0,0)
				ny = -fpga_getPos(1);

				if (nx || ny) {
					*T = 1.0f / center.F_fine; // clocks / mm
					ts = fabs(center.R) * (*T); // clocks

					fpga_setCenterMode(FPGA_CENTER_REVERSE);
					step_writeXYUV(0, ts, nx, ny, 0, 0);

					printf("Move %d %d\n", (int)nx, (int)ny);

					state = ST_REV_FINE0;
					break;
				}
			}
		}

		center_reset();
		center_default();
		return -1;

	case ST_REV_FINE1:
		if (step_isStopped()) {
			if (fpga_getCentered()) {
				pos[1] = fpga_getPos(axis);
				C_acc[axis] += pos[0] + pos[1]; // / 2
				D_acc[axis] += abs(pos[1] - pos[0]);

				if (axis == AXIS_X) {
					nx = 0;
					ny = D_acc[1] ? round((double)D_acc[1] / (acc_cnt << 1)) : mm_to_steps(center.R, cnc_scale_y());

					if (nx || ny) {
						*T = 1.0f / center.F; // clocks / mm
						ts = steps_to_mm(ny, cnc_scale_y()) * (*T); // clocks

						fpga_setCenterMode(FPGA_CENTER_OFF);
						step_writeXYUV(0, ts, nx, ny, 0, 0);

						printf("Mode:Ñ Tx:%d Ty:%d x:%d y:%d\n", (int)(nx ? round(ts/nx) : 0), (int)(ny ? round(ts/ny) : 0), (int)nx, (int)ny);

						axis = AXIS_Y;
						state = ST_CENTER;
						break;
					}
				}
				else if (axis == AXIS_Y) { // next attempt
					if (acc_cnt >= center.attempts) { // finish
						// current
						pt[0].x = fpga_getPos(0);
						pt[0].y = fpga_getPos(1);

						// center
						pt[1].x = round((double)C_acc[AXIS_X]) / (acc_cnt << 1);
						pt[1].y = round((double)C_acc[AXIS_Y]) / (acc_cnt << 1);

						nx = pt[0].x - pt[1].x;
						ny = pt[0].y - pt[1].y;

						if (nx || ny) {
							double k = 1.0 / acc_cnt;
							D[AXIS_X] = round(k * D_acc[AXIS_X]);
							D[AXIS_Y] = round(k * D_acc[AXIS_Y]);

							*T = 1.0f / center.F; // clocks / mm
							ts = sqrt((double)nx * nx + (double)ny * ny) * (*T); // clocks

							fpga_setCenterMode(FPGA_CENTER_OFF);
							step_writeXYUV(0, ts, nx, ny, 0, 0);

							printf("Mode:Ñ Tx:%d Ty:%d x:%d y:%d\n", (int)(nx ? round(ts/nx) : 0), (int)(ny ? round(ts/ny) : 0), (int)nx, (int)ny);

							state = ST_WAIT;
							break;
						}
					}
					else {
						nx = D_acc[0] ? round((double)D_acc[0] / (acc_cnt << 1)) : mm_to_steps(center.R, cnc_scale_x());
						ny = 0;

						if (nx) {
							acc_cnt++;
							*T = 1.0f / center.F; // clocks / mm
							ts = steps_to_mm(nx, cnc_scale_x()) * (*T); // clocks

							fpga_setCenterMode(FPGA_CENTER_OFF);
							step_writeXYUV(0, ts, nx, ny, 0, 0);

							printf("Mode:Ñ Tx:%d Ty:%d x:%d y:%d\n", (int)(nx ? round(ts/nx) : 0), (int)(ny ? round(ts/ny) : 0), (int)nx, (int)ny);

							axis = AXIS_X;
							state = ST_CENTER;
							break;
						}
					}
				}
			}

			center_reset();
			center_default();
			return -1;
		}

	case ST_CENTER:
		if (step_isStopped()) {
			float pct = (100 - center.fineZonePct) * 0.001;
			float R = center.R * pct;
			nx = mm_to_steps(R, cnc_scale_x());
			ny = 0;

			if (nx) {
				*T = 1.0f / center.F; // clocks / mm
				ts = fabs(R) * (*T); // clocks

				fpga_setCenterMode(FPGA_CENTER_FORWARD);
				step_writeXYUV(0, ts, nx, ny, 0, 0);

				printf("Mode:Ñ Tx:%d Ty:%d x:%d y:%d\n", (int)(nx ? round(ts/nx) : 0), (int)(ny ? round(ts/ny) : 0), (int)nx, (int)ny);

				state = ST_FWD0;
				break;
			}

			center_reset();
			center_default();
			return -1;
		}
		break;

	case ST_WAIT:
		if (step_isStopped()) {
			center_reset();
			center_default();
			cnc_reqG92_xyuv_enc(0, 0, 0, 0, 0, 0);
			return 1;
		}
		break;
	}

#ifdef PRINT
	if (state != state_old) {
		switch (state) {
		case ST_IDLE: printf("\tST_IDLE\n"); break;
		case ST_FWD: printf("\tST_FWD\n"); break;
		case ST_FINE: printf("\tST_FINE\n"); break;
		case ST_FWD0: printf("\tST_FWD0\n"); break;
		case ST_FWD_FINE0: printf("\tST_FWD_FINE0\n"); break;
		case ST_REV_FINE0: printf("\tST_REV_FINE0\n"); break;
		case ST_FWD1: printf("\tST_FWD1\n"); break;
		case ST_FWD_FINE1: printf("\tST_FWD_FINE1\n"); break;
		case ST_REV_FINE1: printf("\tST_REV_FINE1\n"); break;
		case ST_CENTER: printf("\tST_CENTER\n"); break;
		case ST_WAIT: printf("\tST_WAIT\n"); break;
		default: printf("\tstate: %d\n", state); break;
		}
	}
#endif

	return 0;
}
