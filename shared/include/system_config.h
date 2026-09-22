#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/**
 *  System configuration that is shared between both cores
 *    Use this for things like specifying which SPI bus is 
 *    used for each peripheral, pins, rates, etc.
 */


 
////////////// SPI INTERFACES ////////////////////
#define SPI_DRV SCB5 // SPI bus to use for motor driver
#define SPI_DRV_CONFIG scb_5_config // SPI configuration for motor driver

#define SPI_PER SCB6 // SPI bus to use for peripheral (encoder, FPGA, etc.)
#define SPI_PER_CONFIG scb_6_config // SPI configuration for peripheral


////////////// PRINTING FUNCTION /////////////////
#define PRINT(...) printf(__VA_ARGS__); __enable_irq();


#endif