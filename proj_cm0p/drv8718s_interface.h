#ifndef DRV8718S_INTERFACE_H
#define DRV8718S_INTERFACE_H

#include <stdbool.h>

/**
 *  Interface code for DRV8718s motor driver
 *    Initializing, configuring registers, 
 *    enabling/disabling driver, etc.
 */

 bool drv8718s_init(void);

#endif