#include "context.h"
#include "fpga_gpo.h"
#include "fpga.h"
#include "cnc_task.h"
#include "feedback.h"
#include "encoder.h"
#include "sem_led.h"
#include "cnc_param.h"

static cnc_context_t cnc_ctx;

cnc_context_t* cnc_ctx_getForce() {
	static fpga_controls_t data32_reg;
	static fpga_lim_switch_t limsw_mask_reg;

	fpga_snapMotor();
	enc_snap(); // TODO: one snapshort for all

	cnc_ctx.field.state = cnc_getState();

	cnc_ctx.field.uv_ena = cnc_getUVEnabled();
	cnc_ctx.field.id = fpga_getTaskID();

	cnc_ctx.field.rev		= cnc_isReverse();
	cnc_ctx.field.rollback	= cnc_isRollback();
	cnc_ctx.field.attempt	= cnc_getRollbackAttempt() & 7;

	cnc_ctx.field.x = fpga_getPos(0);
	cnc_ctx.field.y = fpga_getPos(1);
	cnc_ctx.field.u = fpga_getPos(2);
	cnc_ctx.field.v = fpga_getPos(3);

	cnc_ctx.field.enc_x = enc_get(0);
	cnc_ctx.field.enc_y = enc_get(1);

	data32_reg.data = fpga_getControls();
	limsw_mask_reg.data = fpga_getLimSwMask();
	BOOL hold_ena = fpga_getMotorOE();

	cnc_ctx.field.pump_ena = data32_reg.field.pump_ena;
	cnc_ctx.field.drum_state = data32_reg.field.drum_state;
	cnc_ctx.field.wire_ena = limsw_mask_reg.field.wire_ctrl;
	cnc_ctx.field.voltage_ena = data32_reg.field.voltage_ena;
	cnc_ctx.field.hold_ena = hold_ena;
	cnc_ctx.field.is_init = cnc_isInit();

	cnc_ctx.field.drum_vel = data32_reg.field.drum_vel;
	cnc_ctx.field.voltage_level = data32_reg.field.voltage_level;
	cnc_ctx.field.current_index = getCurrentIndex(data32_reg.data);

	cnc_ctx.field.pulse_width = data32_reg.field.pulse_width;
	cnc_ctx.field.pulse_ratio = data32_reg.field.pulse_ratio;

	cnc_ctx.field.T = cnc_getT();
	cnc_ctx.field.T_cur = cnc_getCurrentT();
	cnc_ctx.field.step = cnc_step();

	uint16_t limsw_reg = fpga_getLimSwReg();
	cnc_ctx.field.limsw_fwd		= (limsw_reg & LIM_FWD_MSK) != 0;
    cnc_ctx.field.limsw_rev		= (limsw_reg & LIM_REV_MSK) != 0;
    cnc_ctx.field.limsw_alm		= (limsw_reg & LIM_ALARM_MSK) != 0;
    cnc_ctx.field.wire_break	= (limsw_reg & LIM_WIRE_MSK) != 0;
    cnc_ctx.field.pwr			= (limsw_reg & LIM_POWER_MSK) != 0;

    uint16_t flags = fpga_getFlags();
    cnc_ctx.field.fb		= (flags & FLAG_NO_PERMIT_MSK) != 0;
    cnc_ctx.field.fb_to		= (flags & FLAG_TIMEOUT_MSK) != 0;
    cnc_ctx.field.hv_ena	= (flags & FLAG_HV_ENA_MSK) != 0;

    uint16_t sem_reg = fpga_readSemReg();
    cnc_ctx.field.sem_ena	= (sem_reg & SEM_ENA_MSK) != 0;
    cnc_ctx.field.sem		= sem_reg & SEM_MSK;
	cnc_ctx.field.fb_ena	= fb_isEnabledForce();
	cnc_ctx.field.attempts	= cnc_getRollbackAttempts() & 7;

	cnc_ctx.field.backup_valid = FALSE;
	cnc_ctx.field.read_valid = TRUE;

	return &cnc_ctx;
}

uint32_t cnc_ctx_get(size_t index) {
	return index < CNC_CONTEX_SIZE32 ? cnc_ctx.data[index] : 0;
}

void cnc_ctx_clear() {
	cnc_ctx.field.read_valid = FALSE;
}
