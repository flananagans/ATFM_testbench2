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

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "pwm_interface.h"
//#include "motor_twophase.h"
//#include "fpga_interface.h"
#include "cycfg_peripherals.h"  /* Include auto-generated configs */
#include "system_config.h"


#define PWM_NEUTRAL_DUTY    0.5f // this is in physical PWM duty ([0,1])
#define PWM_NEUTRAL_COMPARE 1000 // half of 2000 counter period

////////////// VARIABLES ////////////////////////////////////
uint32_t system_uptime_tick = 0; // system time in PWM ticks
static volatile bool trigger_state = false;  // Track PWM state

/////////////// PWM STATE ///////////////////////////////////
#define PWM_CNT_MASK (uint32_t)((1UL << tcpwm_0_cnt_0_NUM) | \
                                (1UL << tcpwm_0_cnt_1_NUM) | \
                                (1UL << tcpwm_0_cnt_4_NUM) | \
                                (1UL << tcpwm_0_cnt_5_NUM) | \
                                (1UL << tcpwm_0_cnt_7_NUM))
static volatile float duty_a = 0.0f;
static volatile float duty_a_half = 0.0f; // half of duty_b
static volatile float duty_b = 0.0f;
static volatile float duty_b_half = 0.0f; // half of duty_b
static volatile uint32_t compare_a1 = PWM_NEUTRAL_COMPARE;
static volatile uint32_t compare_a2 = PWM_NEUTRAL_COMPARE;
static volatile uint32_t compare_b1 = PWM_NEUTRAL_COMPARE;
static volatile uint32_t compare_b2 = PWM_NEUTRAL_COMPARE;

/************* PRIVATE FUNCTION DECLARATIONS **************/

/////////////// STARTUP //////////////////////
void init_pwm_output(void);

/************* PUBLIC FUNCTION DEFINITIONS ***************/

/**
 *  Initialize PWM outputs
 */
bool pwm_init(void) {

  init_pwm_output();
  update_pwm_duty(PWM_NEUTRAL_DUTY, PWM_NEUTRAL_DUTY);

  return true;
}

/**
 *  Update the current PWM output duty cycle
 */
void update_pwm_duty(float da, float db) {

  // all periods should be the same for PWMs 1-4
  uint32_t period = Cy_TCPWM_PWM_GetPeriod0(tcpwm_0_cnt_0_HW, tcpwm_0_cnt_0_NUM);
  //printf("period: %lu, duty a: %.3f, duty b: %.3f\n", period, duty_a, duty_b);

  // compensate for PWM deadzone
  if(da > 0) da += PWM_DEADZONE;
  if(da < 0) da -= PWM_DEADZONE;
  if(db > 0) db += PWM_DEADZONE;
  if(db < 0) db -= PWM_DEADZONE;

  // Clamp duty percentages to correct range on [-1,1] scale
  if (da > PWM_MAX_MODULATION) da = PWM_MAX_MODULATION;
  if (da < -PWM_MAX_MODULATION) da = -PWM_MAX_MODULATION;
  if (db > PWM_MAX_MODULATION) db = PWM_MAX_MODULATION;
  if (db < -PWM_MAX_MODULATION) db = -PWM_MAX_MODULATION;

  duty_a_half = da*0.5f;
  duty_b_half = db*0.5f;

  // Phase A
  compare_a1 = (uint32_t)(((float)period) * (PWM_NEUTRAL_DUTY - duty_a_half));
  compare_a2 = (uint32_t)(((float)period) * (PWM_NEUTRAL_DUTY + duty_a_half));

  // Phase B
  compare_b1 = (uint32_t)(((float)period) * (PWM_NEUTRAL_DUTY - duty_b_half));
  compare_b2 = (uint32_t)(((float)period) * (PWM_NEUTRAL_DUTY + duty_b_half));

  //printf("%lu,%lu,%lu,%lu\n", compare_a1, compare_a2, compare_b1, compare_b2);

  // update PWM compare values
  Cy_TCPWM_PWM_SetCompare0Val(tcpwm_0_cnt_4_HW, tcpwm_0_cnt_4_NUM, compare_a1);
  Cy_TCPWM_PWM_SetCompare0Val(tcpwm_0_cnt_5_HW, tcpwm_0_cnt_5_NUM, compare_a2);
  Cy_TCPWM_PWM_SetCompare0Val(tcpwm_0_cnt_7_HW, tcpwm_0_cnt_7_NUM, compare_b1);
  Cy_TCPWM_PWM_SetCompare0Val(tcpwm_0_cnt_0_HW, tcpwm_0_cnt_0_NUM, compare_b2);
}

/************* PRIVATE FUNCTION DECLARATIONS **************/
/**
 *  Initialize PWM outputs simultaneously
 */
void init_pwm_output(void) {

  // Counter used to measure differences in time between ISR cycles
  Cy_TCPWM_Counter_Init(tcpwm_0_cnt_1_HW, tcpwm_0_cnt_1_NUM, &tcpwm_0_cnt_1_config);

  // Drive for phase A HB1
  Cy_TCPWM_PWM_Init(tcpwm_0_cnt_4_HW, tcpwm_0_cnt_4_NUM, &tcpwm_0_cnt_4_config);
  // Drive for phase A HB2
  Cy_TCPWM_PWM_Init(tcpwm_0_cnt_5_HW, tcpwm_0_cnt_5_NUM, &tcpwm_0_cnt_5_config);
  // Drive for phase B HB1
  Cy_TCPWM_PWM_Init(tcpwm_0_cnt_7_HW, tcpwm_0_cnt_7_NUM, &tcpwm_0_cnt_7_config);
  // Drive for phase B HB2
  Cy_TCPWM_PWM_Init(tcpwm_0_cnt_0_HW, tcpwm_0_cnt_0_NUM, &tcpwm_0_cnt_0_config);

  // Enable all simultaneously
  Cy_TCPWM_Enable_Multiple(TCPWM0, PWM_CNT_MASK);
  // Trigger all simultaneously
  Cy_TCPWM_TriggerStart(TCPWM0, PWM_CNT_MASK);
}