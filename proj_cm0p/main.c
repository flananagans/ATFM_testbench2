/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for CM0+ in the the Dual CPU Empty 
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/
#include "cyhal.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

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


int main(void) {
    /* Enable global interrupts */
    __enable_irq();
    
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    /* Enable CM4. CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR);

    /* Retarget stdin and stdout to the debug UART port */
    result = cy_retarget_io_init(P5_5, P5_4, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    PRINT("Hello from CM0!\r\n");

    init_spi();
    test_spi();

    for (;;) {
    }

}

/* [] END OF FILE */
