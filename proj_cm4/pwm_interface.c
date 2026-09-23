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

#include "motor_twophase.h"
#include "fpga_interface.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cycfg_peripherals.h"  /* Include auto-generated configs */
#include "system_config.h"


#define PWM_NEUTRAL_DUTY    0.5f // this is in physical PWM duty ([0,1])
#define PWM_NEUTRAL_COMPARE 1000 // half of 2000 counter period

/////////////// SYSTEM STATE ////////////////////////////////
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
static void init_pwm_output(void);
//static void pwm_tc_isr(void);
//void init_timer_interrupt(void);

/////////////// PWM UTILITIES ///////////////
//void pwm_update_callback(void);

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
 *  Primary control callback for the CM4. 
 *  Increments our clock, drives encoder readings,
 *  motor control, and updating PWM
 */
 /*
void pwm_update_callback(void) {

  // Increment time
  system_uptime_tick++;

  // Trigger encoder reading
  trigger_state = !trigger_state;
  cyhal_gpio_write(FPGA_TRIGGER_PIN, trigger_state);
  
  // Start next FPGA async transfer only if no unread result is pending 
  // TEST -> does this still work if SPI request is on falling edge of trigger?
  if (!trigger_state) {
    if (!fpga_is_busy() && !fpga_transfer_complete()) {
      (void)fpga_request_data();
    }
  } else {
    fpga_set_last_trigger_time(system_uptime_tick);
  }

  // Parse new FPGA data if available
	(void)fpga_update_data();

  if ((!driver_enabled) || (motor_get_mode() == MOTOR_MODE_DISABLED)) {
    return;
  }

  motor_calculate_control(&duty_a, &duty_b);

  update_pwm_duty(duty_a, duty_b);

}
*/

/**
 *  Update the current PWM output duty cycle
 */
void update_pwm_duty(float da, float db)
{
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
static void init_pwm_output(void) {

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

/**
 *  TCPWM Terminal Count Interrupt Handler 
 */
/*
static void pwm_tc_isr(void) {
    // Clear the Terminal Count interrupt flag for Counter 0
    Cy_TCPWM_ClearInterrupt(tcpwm_0_cnt_0_HW, tcpwm_0_cnt_0_NUM, CY_TCPWM_INT_ON_TC);
    
    // Run control loop
    pwm_update_callback();
}
*/

/**
 *  Initialize and start the PWM interrupt service
 */
 /*
void init_timer_interrupt(void) {

    system_uptime_tick = 0;

    // Define the System Interrupt configuration
    // Note: 'tcpwm_0_interrupts_0_IRQn' is the standard naming convention. 
    //   If your IDE complains, check your cycfg_peripherals.h for the exact IRQn macro 
    //   assigned to your TCPWM block (it might be named something like ioss_interrupts_IRQn 
    //   depending on your specific PSoC 6 routing). 
    const cy_stc_sysint_t pwm_int_config = {
        .intrSrc      = tcpwm_0_interrupts_0_IRQn, 
        .intrPriority = 2UL  // High priority for tight motor control
    };

    // 3. Initialize and enable the interrupt in the NVIC
    Cy_SysInt_Init(&pwm_int_config, pwm_tc_isr);
    NVIC_EnableIRQ(pwm_int_config.intrSrc);

    // 4. Tell the TCPWM hardware to generate a signal on Terminal Count (TC)
    Cy_TCPWM_SetInterruptMask(tcpwm_0_cnt_0_HW, tcpwm_0_cnt_0_NUM, CY_TCPWM_INT_ON_TC);
}
*/