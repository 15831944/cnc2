#ifndef MY_TYPES_H
#define MY_TYPES_H

#include <cstdint>

enum AXIS {AXIS_X, AXIS_Y, AXIS_NUM};
enum DIR {PLUS, MINUS};

enum class SLICING_PROFILE : uint8_t { RECT, CIRCLE };

struct slicing_t {
    SLICING_PROFILE profile {SLICING_PROFILE::RECT};
    unsigned slices_num {0}, sections_num {0};
    double wireD {0}, width {0}, spacing {0}, slicing_step {0}, speed_avg {0};
};

struct comb_t {
    AXIS axis;
    uint8_t drum_vel {0};
    unsigned teeth_num {0};
    double wireD {0}, depth {0}, step {0}, speed {0}, speed_idle {0};
};

#endif // MY_TYPES_H
