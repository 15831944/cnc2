#include "uv.h"
#include <math.h>
#include <stdio.h>
#include "aux_func.h"
#include "cnc_func.h"

static double L; // distance between XY_motor and UV_motor planes, mm
static double H; // height to XY from XY_motor plane, mm
static double D; // thickness, mm
static BOOL valid;

int32_t u_max = (int32_t)INT32_MAX, u_min = (int32_t)INT32_MIN;
int32_t v_max = (int32_t)INT32_MAX, v_min = (int32_t)INT32_MIN;

void uv_setL(double value) { L = value > 0 ? value : 0; valid = FALSE; }
double uv_getL() { return L; }

void uv_setH(double value) { H = value > 0 ? value : 0; valid = FALSE; }
double uv_getH() { return H; }

void uv_setD(double value) { D = value > 0 ? value : 0; valid = FALSE; }
double uv_getD() { return D; }

BOOL uv_applyParameters() {
	if (L > 0 && H > 0 && D > 0 && H + D < L)
		valid = TRUE;
	else
		uv_clear();

	return valid;
}

void uv_clear() {
	L = 0;
	H = 0;
	D = 0;
	valid = FALSE;
}

void uv_defaultParam() {
	L = UV_L;
	H = UV_H;
	D = UV_D;
	valid = TRUE;
}

BOOL uv_valid() { return valid; }

static double uv_line_x(double x0, double x1, double z0, double z1, double z) {
	if (!valid || z1 == z0)
		return x0;

	return (x1 - x0) / (z1 - z0) * (z - z0) + x0;
}

// XY - Z = 0
double uv_X_to_motor(double X, double U) {
	double res = uv_line_x(X, U, 0, D, -H);
	return res;
}
double uv_U_to_motor(double X, double U) {
	double Um = uv_line_x(X, U, 0, D, L - H);
	double Xm = uv_X_to_motor(X, U);
	double res = Um - Xm;
	return res;
}
double uv_Y_to_motor(double Y, double V) {
	double res = uv_line_x(Y, V, 0, D, -H);
	return res;
}
double uv_V_to_motor(double Y, double V) {
	double Vm = uv_line_x(Y, V, 0, D, L - H);
	double Ym = uv_Y_to_motor(Y, V);
	double res = Vm - Ym;
	return res;
}

double uv_motor_to_X(double Xm, double Um)	{ return uv_line_x(Xm, Xm + Um, -H, L - H, 0); }
double uv_motor_to_U(double Xm, double Um)	{ return uv_line_x(Xm, Xm + Um, -H, L - H, D); }
double uv_motor_to_Y(double Ym, double Vm)	{ return uv_line_x(Ym, Ym + Vm, -H, L - H, 0); }
double uv_motor_to_V(double Ym, double Vm)	{ return uv_line_x(Ym, Ym + Vm, -H, L - H, D); }

fpoint_t uv_XY_to_motors(const fpoint_t* const XY, const fpoint_t* const UV) {
	fpoint_t XYm = {uv_X_to_motor(XY->x, UV->x), uv_Y_to_motor(XY->y, UV->y)};
	return XYm;
}

fpoint_t uv_UV_to_motors(const fpoint_t* const XY, const fpoint_t* const UV) {
	fpoint_t UVm = {uv_U_to_motor(XY->x, UV->x), uv_V_to_motor(XY->y, UV->y)};
	return UVm;
}

fpoint_t uv_motors_to_XY(const fpoint_t* const XYm, const fpoint_t* const UVm) {
	fpoint_t XY;
	XY.x = uv_motor_to_X(XYm->x, UVm->x);
	XY.y = uv_motor_to_Y(XYm->y, UVm->y);
	return XY;
}

fpoint_t uv_motors_to_UV(const fpoint_t* const XYm, const fpoint_t* const UVm) {
	fpoint_t UV;
	UV.x = uv_motor_to_U(XYm->x, UVm->x);
	UV.y = uv_motor_to_V(XYm->y, UVm->y);
	return UV;
}

point_t uv_limit(point_t pt) {
	if (pt.x > u_max)
		pt.x = u_max;
	if (pt.x < u_min)
		pt.x = u_min;

	if (pt.y > v_max)
		pt.y = v_max;
	if (pt.y < v_min)
		pt.y = v_min;

	return pt;
}

void uv_tb() {
	uv_setL( 150 );
	uv_setH( 50 );
	uv_setD( 30 );
	uv_applyParameters();

	fpoint_t XY = {10, 5};
	fpoint_t UV = {15, 6};

	fpoint_t XYm = uv_XY_to_motors(&XY, &UV);
	fpoint_t UVm = uv_UV_to_motors(&XY, &UV);

	fpoint_t XY2 = uv_motors_to_XY(&XYm, &UVm);
	fpoint_t UV2 = uv_motors_to_UV(&XYm, &UVm);

//	decimal_t x = float2fix(XY.x);
//	decimal_t y = float2fix(XY.y);
//	decimal_t u = float2fix(UV.x);
//	decimal_t v = float2fix(UV.y);

	decimal_t x2 = float2fix(XY2.x);
	decimal_t y2 = float2fix(XY2.y);
	decimal_t u2 = float2fix(UV2.x);
	decimal_t v2 = float2fix(UV2.y);

	printf("[%d.%d, %d.%d, %d.%d, %d.%d]\n", x2.value, x2.rem, y2.value, y2.rem, u2.value, u2.rem, v2.value, v2.rem);
}
