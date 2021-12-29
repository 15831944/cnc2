#ifndef INC_CENTER_H_
#define INC_CENTER_H_

#include "my_types.h"
#include "fpga.h"

#define CENTER_ATTEMPTS_MAX (5)
#define CENTER_RADIUS_MAX (1000.0f) // mm

typedef enum {CENTER_NO, CENTER_X, CENTER_Y, CENTER_CIRCLE} CENTER_MODE_T;

typedef struct {
	uint8_t attempts, fineZonePct, drumVel;
	CENTER_MODE_T mode;
	float R, rollback, F, F_fine;
} center_t;

int center_init(const center_t* const center);
int center_next(float* const T);

int32_t center_D(AXIS_T axis);

#endif /* INC_CENTER_H_ */
