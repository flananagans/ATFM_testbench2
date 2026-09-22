/**
 *  Interface code for DRV8718s motor driver
 *    Initializing, configuring registers, 
 *    enabling/disabling driver, etc.
 */

#include "cyhal.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "cybsp.h"
#include "drv8718s_interface.h"
#include "system_config.h"


void init_spi(void) {

    cy_en_scb_spi_status_t init_status = Cy_SCB_SPI_Init(SPI_DRV, &SPI_DRV_CONFIG, NULL); // context not required for low-level API

    if(init_status != CY_SCB_SPI_SUCCESS) {
        PRINT("SPI INIT FAILED\r\n");
    }

    Cy_SCB_SPI_SetActiveSlaveSelect(SPI_DRV, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable SPI to operate */
    Cy_SCB_SPI_Enable(SPI_DRV);
    __enable_irq();
}

void test_spi(void) {
    uint8_t txBuffer[3];

    /* Initialize txBuffer with command to transfer */
    txBuffer[0] = 0x00U;
    txBuffer[1] = 0x00U;
    txBuffer[2] = 0x00U;
    
    PRINT("SPI transfer start!\r\n");

    /* Start transfer */
    Cy_SCB_SPI_WriteArrayBlocking(SPI_DRV, txBuffer, sizeof(txBuffer));
    
    /* Blocking wait for transfer completion */
    while (!Cy_SCB_SPI_IsTxComplete(SPI_DRV))
    {
        PRINT("SPI transferring...\r\n");
    }

    PRINT("SPI transfer done!\r\n");

}


bool drv8718s_init(void) {

  // Initialize SPI bus
  init_spi();
  test_spi();

  return true;
}

