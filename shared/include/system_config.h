#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/**
 *  System configuration that is shared between both cores
 *    Use this for things like specifying which SPI bus is 
 *    used for each peripheral, pins, rates, etc.
 */

#include <stdio.h>
#include <stdint.h>

////////////// SYSTEM STATE //////////////////////
uint32_t system_uptime_tick = 0; // system time in PWM ticks
bool driver_enabled = 0; // 1 for enabled
uint32_t driver_status = 0; // [0,0,STAT1,STAT2] 
bool driver_fault = 0; // 1 for fault
 
////////////// MOTOR SPECS ///////////////////////
#define MOTOR_PHASES             2
#define SINE_SAMPLES             3600
#define MOTOR_POLE_PAIRS         24.0f
#define MOTOR_COM_REF            194.33f // for reassembled Syd motor 09042026
#define MOTOR_KT                 0.15f // motor coil Kt
#define MOTOR_COIL_RES           0.08f //0.08ohm coil resistance
#define MOTOR_V_BUS              15.0f // driver bus voltage
#define MOTOR_DAMPING_CCW        0.153f // motor damping coefficient ccw direction (Nm/s)
#define MOTOR_DAMPING_CW         0.074f // motor damping coefficient cw direction (Nm/s)

///////////// PWM SPECS /////////////////////////
// this is in desired duty [-1,1] -- 0.5f means max is [-0.5, +0.5]
#define PWM_MAX_MODULATION  0.75f  
// PWM deadzone amplitude -- 0.0168 means [-0.0168, 0.0168] will be jumped over
#define PWM_DEADZONE        0.045f; 

///////////// ENCODER SPECS //////////////////////
#define ENC_LATENCY_US      78      // us between encoder measurement and FPGA read
#define ENCODER_RESOLUTION  524288.0f  /* 19-bit encoder = 524288 counts/rev */
#define DEGREES_PER_COUNT   (360.0f / ENCODER_RESOLUTION)
#define FPGA_TRIGGER_PIN    P10_0

///////////// TIMING SPECS ///////////////////////
#define PWM_UPDATE_RATE_HZ  25000   // 25 kHz update rate
#define PWM_TICK_US         40      // us per 25kHz PWM cycle
#define MS_TO_TICKS(ms)     ((ms) * (PWM_UPDATE_RATE_HZ / 1000))

////////////// SPI INTERFACES ////////////////////
#define SPI_DRV SCB5 // SPI bus to use for motor driver
#define SPI_DRV_CONFIG scb_5_config // SPI configuration for motor driver

#define SPI_PER SCB6 // SPI bus to use for peripheral (encoder, FPGA, etc.)
#define SPI_PER_CONFIG scb_6_config // SPI configuration for peripheral

///////////// DRV8718s PINS //////////////////////
/* Control Signals (GPIO) */
#define DRV_NSLEEP_PIN              P9_3    /* Pin 8: nSLEEP */
#define DRV_NSLEEP_PORT             GPIO_PRT9
#define DRV_NSLEEP_PIN_NUM          3

#define DRV_DRVOFF_nFLT_PIN         P9_4    /* Pin 9: DRVOFF/nFLT */
#define DRV_DRVOFF_nFLT_PORT        GPIO_PRT9
#define DRV_DRVOFF_nFLT_PIN_NUM     4

#define DRV_BRAKE_PIN               P9_5    /* Pin 14: BRAKE */
#define DRV_BRAKE_PORT              GPIO_PRT9
#define DRV_BRAKE_PIN_NUM           5


///////////// CONSTANTS //////////////////////////
#define PI                  3.14159f
#define DEG_TO_RAD          0.01745329252f
#define RAD_TO_DEG          180.0f/PI

////////////// PRINTING FUNCTION /////////////////
#define PRINT(...) printf(__VA_ARGS__); __enable_irq();


#endif