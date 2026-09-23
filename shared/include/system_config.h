#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/**
 *  System configuration that is shared between both cores
 *    Use this for things like specifying which SPI bus is 
 *    used for each peripheral, pins, rates, etc.
 */

#include <stdio.h>
 
////////////// MOTOR SPECS ///////////////////////
#define MOTOR_PHASES             2
#define SINE_SAMPLES             3600
#define MOTOR_POLE_PAIRS         24.0f
#define MOTOR_COM_REF            194.33f // for reassembled Syd motor 09042026

///////////// ENCODER SPECS //////////////////////
#define ENC_LATENCY_US      78      // us between encoder measurement and FPGA read


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

////////////// PRINTING FUNCTION /////////////////
#define PRINT(...) printf(__VA_ARGS__); __enable_irq();


#endif