#include "cnc_context.h"
#include <cstdio>

void toDebug(const cnc_context_t * const ctx) {
    qDebug("pump:%x drumSt:%x wire:%x hv:%x hold:%x\n",
           ctx->field.pump_ena, ctx->field.drum_state, ctx->field.wire_ena, ctx->field.voltage_ena, ctx->field.hold_ena);
    qDebug("drumVel:%x\n",
           ctx->field.drum_vel);
    qDebug("uv:%x rev:%x enc:%x\n",
           ctx->field.uv_ena, ctx->field.rev, ctx->field.enc_ena);
    qDebug("St:%x\n",
           ctx->field.state);
    qDebug("pW:%d pR:%d hvLvl:%x I:%x\n",
           ctx->field.pulse_width, ctx->field.pulse_ratio, ctx->field.voltage_level, ctx->field.current_index);
    qDebug("ID:%d X:%d Y:%d U:%d V:%d\n",
           (int)ctx->field.id, (int)ctx->field.x, (int)ctx->field.y, (int)ctx->field.u, (int)ctx->field.v);
    qDebug("encX:%d encY:%d\n",
           (int)ctx->field.enc_x, (int)ctx->field.enc_y);
    qDebug("T:%g\n",
           ctx->field.T);
    qDebug("T:%g\n",
           ctx->field.step);
    qDebug("bkp:%x read:%x\n",
           ctx->field.backup_valid, ctx->field.read_valid);
}

#ifdef CNC2
const double cnc_adc_volt_t::coe[6] {
    volt(1.0 / 61.04, 4.096, 10),
    volt(1.0 / 111.08, 4.096, 10),
    volt(1e6, 20e3, 1, 4.096, 10),
    volt(1e6, 20e3, 1, 4.096, 10),
    volt(1e6, 20e3, 1, 4.096, 10),
    volt(10, 4.096, 10),
};
#else
//const double cnc_adc_volt_t::coe[6] {
//    volt(100e3, 3.01e3, 1, 4.096, 10), // diff
//    volt(10e3, 100e3, 10, 4.096, 10), // shunt
//    volt(100e3, 3.01e3, 1, 4.096, 10), // back
//    volt(100e3, 3.01e3, 1, 4.096, 10), // HV
//    volt(100e3, 3.01e3, 1, 4.096, 10), // Workpiece+
//    volt(100e3, 3.01e3, 1, 4.096, 10) // Wire-
//};
#endif

std::string CncContext::toStringCenterDebug(double dia) const {
    cnc_state_t state = static_cast<cnc_state_t>(m_context.field.state);
    center_state_t center_state = static_cast<center_state_t>(m_context.field.center_state);
    touch_state_t touch_state = static_cast<touch_state_t>(m_context.field.touch_state);
    FPGA_CENTER_MODE_T center_mode = static_cast<FPGA_CENTER_MODE_T>(m_context.field.center_mode);

    std::string s = "State: "       + stateToString(state) + "\t Center: " + stateToString(center_state) + "\n" +
            "Touch: " + stateToString(touch_state) + "\t Mode: " + stateToString(center_mode) + "\n" +
            "Feedback: "    + (m_context.field.fb_ena ? "(Yes)" : "(no)") + " " +
            "Weak HV: "     + (m_context.field.center_ena ? "(Yes)" : "(no)") + " " +
            "Attempt: "     + std::to_string(m_context.field.center_attempt) + " (" + std::to_string(m_context.field.center_attempts) + ")" + " " +
            "Touch: "       + std::to_string(m_context.field.touch) + " (" + std::to_string(m_context.field.touches) + ")";
    if (dia >= 0) {
        char buf[100];

        dia = round(dia * 1e3) / 1e3;
        snprintf(buf, sizeof(buf), "Diameter: %.3f mm", dia);

        s += "\n";
        s += buf;
    }

    return s;
}
