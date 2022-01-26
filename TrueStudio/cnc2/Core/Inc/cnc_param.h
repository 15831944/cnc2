#ifndef INC_CNC_PARAM_H_
#define INC_CNC_PARAM_H_

#include "my_types.h"
#include "defines.h"
#include "feedback.h"

#define ACC ( 100.0 * COE_UMSEC2_TO_MMTICK2 ) // 100 um/sec2
#define DEC ( 100.0 * COE_UMSEC2_TO_MMTICK2 ) // 100 um/sec2

float velocity(float v0, float acc, float d);

void cnc_resetParam();

void cnc_setStep(double value);
double cnc_step();

void cnc_setAcc(float value);
void cnc_setDec(float value);

float cnc_getAcc();
float cnc_getDec();

float cnc_acc();
float cnc_dec();

float cnc_pidAcc();
float cnc_pidDec();

const scale_t* cnc_scale_xy();
double cnc_scale_x();
double cnc_scale_y();
double cnc_getScale(AXIS_T axis);

const scale_t* cnc_scale_uv();
double cnc_scale_u();
double cnc_scale_v();

const scale_t* cnc_scale_enc();
double cnc_scale_enc_x();
double cnc_scale_enc_y();

#endif /* INC_CNC_PARAM_H_ */
