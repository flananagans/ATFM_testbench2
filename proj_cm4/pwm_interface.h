#ifndef PWM_INTERFACE_H
#define PWM_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>
#include "cy_pdl.h"

/**
 *  Interface code for configuring and updating
 *   center-aligned PWM signals used for controlling
 *   the motor driver
 *
 *  Because we are driving 2 full H-bridges, we have
 *   4 PWMs (2 high side MOSFETs for each H-bridge)
 *
 *   A1 & A2 - high sides phase A (1 causes ccw rotation)
 *   B1 & B2 - high sides phase B (1 causes ccw rotation)
 */ 

/////////////// STARTUP //////////////////////
bool pwm_init(void);

/////////////// RUNNING //////////////////////
//void pwm_update_callback(void); // primary control callback
void update_pwm_duty(float duty_a_pct, float duty_b_pct);

#endif