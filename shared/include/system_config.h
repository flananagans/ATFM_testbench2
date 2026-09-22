#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/**
 *  System configuration that is shared between both cores
 *    Use this for things like specifying which SPI bus is 
 *    used for each peripheral, pins, rates, etc.
 */


 
////////////// SPI INTERFACES ////////////////////
#define SPI_DRV SCB5
#define SPI_PER SCB6

#define PRINT(...) printf(__VA_ARGS__); __enable_irq();


#endif