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

/************* PUBLIC VARIABLES DECLARATIONS **************/
bool driver_enabled = false;
bool driver_fault = false;
uint32_t driver_status = 0; // [0,0,STAT1,STAT2] 

/************* PRIVATE FUNCTION DECLARATIONS **************/

/////////////// STARTUP //////////////////////
bool drv8718s_configure();
void drv8718s_init_gpio();
void drv8718s_configure_nflt_pin(void);
void drv8718s_configure_pwm_mapping(void);
void drv8718s_enable_and_configure(void);
void drv8718s_configure_vds_threshold(void);
void drv8718s_disable_vds_monitoring(void);

/////////////// SPI UTILITIES ///////////////
bool init_spi(void);
void drv8718s_spi_transaction(uint8_t *tx_data, uint8_t *rx_data, size_t length);
uint8_t drv8718s_read_register(uint8_t addr);
void drv8718s_write_register(uint8_t addr, uint8_t data);
uint8_t drv8718s_read_status(void);
uint8_t drv8718s_read_status2(void);
void drv8718s_dump_registers(void);

/************* PUBLIC FUNCTIONS **************/
/** 
 * Initialize and configure the motor driver
 */
bool drv8718s_init(void) {

  // Initialize SPI bus
  if(!init_spi()) {
    return false;
  }
  
  PRINT("SPI started!\r\n");

  if(!drv8718s_configure()) {
    return false;
  }

  return true;
}

/**
 * Enable the driver
 */
void drv8718s_enable(void) {
    driver_enabled = true;
    cyhal_gpio_write(DRV_DRVOFF_nFLT_PIN, 0); // DRVOFF low - enable driver
}

/**
 * Disable the driver
 */
void drv8718s_disable(void) {
    cyhal_gpio_write(DRV_DRVOFF_nFLT_PIN, 1); // nDRVOFF high - disable driver
    driver_enabled = false;
}

/**
 * Clear driver faults
 */
void drv8718s_clear_faults(void) {
    uint8_t ic_ctrl1 = drv8718s_read_register(DRV8718S_REG_IC_CTRL1);
    ic_ctrl1 |= IC_CTRL1_CLR_FLT;
    drv8718s_write_register(DRV8718S_REG_IC_CTRL1, ic_ctrl1);
    Cy_SysLib_Delay(10);
}

/** 
 * Check status of driver. Returns true if fault detected
 */
bool drv8718s_check_status(void) {

    uint8_t stat1 = drv8718s_read_status();

    driver_fault = (stat1 & IC_STAT1_FAULT_MASK) > 0;
    driver_status = (stat1 << 8) | (drv8718s_read_status2());

    return driver_fault;
}

/**
 * Print out register values
 */
void drv8718s_dump_registers(void) {
    struct {
        uint8_t addr;
        const char *name;
    } registers[] = {
        {0x00, "IC_STAT1"}, {0x01, "VDS_STAT1"}, {0x02, "VDS_STAT2"},
        {0x03, "VGS_STAT1"}, {0x04, "VGS_STAT2"}, {0x07, "IC_CTRL1"},
        {0x08, "IC_CTRL2"}, {0x09, "BRG_CTRL1"}, {0x0B, "PWM_CTRL1"},
        {0x0D, "PWM_CTRL3"}, {0x0E, "PWM_CTRL4"}, {0x18, "DRV_CTRL1"}
    };

    PRINT("\n===== DRV8718S Register Dump =====\r\n");
    for (int i = 0; i < 12; i++) {
        uint8_t value = drv8718s_read_register(registers[i].addr);
        PRINT("  %s (0x%02X) = 0x%02X\r\n", registers[i].name, registers[i].addr, value);
    }
    PRINT("===================================\r\n\n");
}

/************* PRIVATE FUNCTION DEFINITIONS **************/
/**
 * Configure the registers of the motor driver
 */
bool drv8718s_configure() {

    // Initialize GPIO control pins
    drv8718s_init_gpio();

    drv8718s_clear_faults();

    // Configure PWM mapping
    drv8718s_configure_pwm_mapping();

    // Configure NFAULT pin
    drv8718s_configure_nflt_pin();

    // Disable VDS monitoring
    drv8718s_disable_vds_monitoring();

    drv8718s_clear_faults();

    drv8718s_enable_and_configure();

    return true;
}

/**
 * Initialize GPIO pins used for the DRV8718s motor driver
 */
void drv8718s_init_gpio(void)
{
    // nSleep pin - flippinf low puts the driver in sleep mode
    cyhal_gpio_init(DRV_NSLEEP_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
    cyhal_gpio_write(DRV_NSLEEP_PIN, 1);  /* nSLEEP HIGH */
    
    // DRVOFF pin - flipping low enables the drives
    cyhal_gpio_init(DRV_DRVOFF_nFLT_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    cyhal_gpio_write(DRV_DRVOFF_nFLT_PIN, 1);  /* DRVOFF LOW = enabled */
    
    // Brake - flipping high shorts motor lead when drive is in sleep state (nSLEEP = low)
    cyhal_gpio_init(DRV_BRAKE_PIN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    cyhal_gpio_write(DRV_BRAKE_PIN, 0);  /* BRAKE LOW = disabled */
}

/**
 * Configure control mapping of the PWM inputs to the motor driver
 */
void drv8718s_configure_pwm_mapping(void) {
    drv8718s_clear_faults();

    uint8_t ic_ctrl1 = IC_CTRL1_EN_OLSC | IC_CTRL1_LOCK | IC_CTRL1_CLR_FLT;
    drv8718s_write_register(DRV8718S_REG_IC_CTRL1, ic_ctrl1);

    // Set bridge configuration
    drv8718s_write_register(DRV8718S_REG_IC_CTRL2, 0xAF);
    drv8718s_write_register(DRV8718S_REG_BRG_CTRL1, 0xFF);
    drv8718s_write_register(DRV8718S_REG_BRG_CTRL2, 0x00);

    // Set PWM inputs and configuration
    uint8_t pwm_ctrl1 = (0x00 << 6) | (0x01 << 4) | (0x02 << 2) | (0x03 << 0);
    drv8718s_write_register(DRV8718S_REG_PWM_CTRL1, pwm_ctrl1);
    drv8718s_write_register(DRV8718S_REG_PWM_CTRL2, 0xAF);
    drv8718s_write_register(DRV8718S_REG_PWM_CTRL3, 0x00);
    drv8718s_write_register(DRV8718S_REG_PWM_CTRL4, 0x00);

    // Set Idrive and tdrive registers
    drv8718s_write_register(DRV8718S_REG_IDRV_CTRL1, (IDRV_CTRL_48MA << 4) | IDRV_CTRL_48MA);
    drv8718s_write_register(DRV8718S_REG_IDRV_CTRL2, (IDRV_CTRL_48MA << 4) | IDRV_CTRL_48MA);
    drv8718s_write_register(DRV8718S_REG_IDRV_CTRL3, (IDRV_CTRL_48MA << 4) | IDRV_CTRL_48MA);
    drv8718s_write_register(DRV8718S_REG_IDRV_CTRL4, (IDRV_CTRL_48MA << 4) | IDRV_CTRL_48MA);

    drv8718s_write_register(DRV8718S_REG_DRV_CTRL2, (VGS_TDRV_2US << 3) | VGS_TDRV_2US);
    drv8718s_write_register(DRV8718S_REG_DRV_CTRL3, (VGS_TDRV_2US << 3) | VGS_TDRV_2US);

    // Disable duty cycle compensation
    drv8718s_write_register(DRV8718S_REG_DCC_CTRL1, 0b00000000);
}

/**
 * Disable monitoring of voltage drain-source current
 */
void drv8718s_disable_vds_monitoring(void)
{
    drv8718s_clear_faults();
    drv8718s_write_register(DRV8718S_REG_DRV_CTRL1, 0xEF);
    drv8718s_clear_faults();
}

/**
 * Configure voltage drain-source monitoring thresholds
 */
void drv8718s_configure_vds_threshold(void)
{
    drv8718s_clear_faults();
    drv8718s_write_register(DRV8718S_REG_VDS_CTRL1, 0xFF);
    drv8718s_write_register(DRV8718S_REG_VDS_CTRL2, 0xFF);
    drv8718s_write_register(DRV8718S_REG_VDS_CTRL3, 0xFF);
    drv8718s_write_register(DRV8718S_REG_VDS_CTRL4, 0xFF);

    // Set PVDD undervoltage mode to automatic recovery
    drv8718s_write_register(DRV8718S_REG_UVOV_CTRL, 0b10010100);
}

/**
 * Configure the nFAULT pin in SPI registers
 */
void drv8718s_configure_nflt_pin(void){
    uint8_t ic_ctrl2 = drv8718s_read_register(DRV8718S_REG_IC_CTRL2);
    ic_ctrl2 |= IC_CTRL2_DRVOFF_NFLT;
    drv8718s_write_register(DRV8718S_REG_IC_CTRL2, ic_ctrl2);
};

void drv8718s_enable_and_configure(void)
{
    drv8718s_clear_faults();

    uint8_t ic_ctrl1 = drv8718s_read_register(DRV8718S_REG_IC_CTRL1);
    ic_ctrl1 |= IC_CTRL1_EN_DRV;
    drv8718s_write_register(DRV8718S_REG_IC_CTRL1, ic_ctrl1);
    Cy_SysLib_Delay(10);

    drv8718s_clear_faults();
}


//////////////////// SPI UTILITIES ////////////////////
/** 
 * Initialize SPI bus for the motor driver
 */
bool init_spi(void) {

    cy_en_scb_spi_status_t init_status = Cy_SCB_SPI_Init(SPI_DRV, &SPI_DRV_CONFIG, NULL); // context not required for low-level API

    if(init_status != CY_SCB_SPI_SUCCESS) {
        return false;
    }

    Cy_SCB_SPI_SetActiveSlaveSelect(SPI_DRV, CY_SCB_SPI_SLAVE_SELECT0);

    /* Enable SPI to operate */
    Cy_SCB_SPI_Enable(SPI_DRV);
    __enable_irq();

    return true;
}

/**
 * SPI transaction utility (blocking)
 */
void drv8718s_spi_transaction(uint8_t *tx_data, uint8_t *rx_data, size_t length)
{
    /* Clear FIFOs*/
    Cy_SCB_ClearTxFifo(SPI_DRV); // clear buffer before status
    Cy_SCB_SPI_ClearTxFifoStatus(SPI_DRV, CY_SCB_SPI_TX_INTR_MASK); // clear all statuses
    Cy_SCB_ClearRxFifo(SPI_DRV); // clear buffer before status
    Cy_SCB_SPI_ClearRxFifoStatus(SPI_DRV, CY_SCB_SPI_RX_INTR_MASK); // clear all statuses

    /* Start transfer */
    Cy_SCB_SPI_WriteArrayBlocking(SPI_DRV, tx_data, length);
    
    /* Blocking wait for transfer completion */
    while (!Cy_SCB_SPI_IsTxComplete(SPI_DRV)) {
        CyDelayUs(100);
    }

    /* Read received data */
    Cy_SCB_ReadArrayBlocking(SPI_DRV, rx_data, length);
    
    return;
}

/**
 * Write to a register on the DRV8718s and delay after
 */
void drv8718s_write_register(uint8_t addr, uint8_t data)
{
    uint16_t cmd = DRV8718S_SPI_WRITE_BIT |
                   ((addr & 0x3F) << DRV8718S_ADDR_SHIFT) |
                   (data & 0xFF);

    uint8_t tx_buffer[2] = {(cmd >> 8) & 0xFF, cmd & 0xFF};
    uint8_t rx_buffer[2] = {0, 0};
	
    drv8718s_spi_transaction(tx_buffer, rx_buffer, 2);
    Cy_SysLib_DelayUs(100);
}

/**
 * Read a register on the DRV8718s
 */
uint8_t drv8718s_read_register(uint8_t addr) {
    uint16_t cmd = DRV8718S_SPI_READ_BIT | ((addr & 0x3F) << DRV8718S_ADDR_SHIFT);
    uint8_t tx_buffer[2] = {(cmd >> 8) & 0xFF, cmd & 0xFF};
    uint8_t rx_buffer[2] = {0, 0};

    
    drv8718s_spi_transaction(tx_buffer, rx_buffer, 2);

    uint16_t response = ((uint16_t)rx_buffer[0] << 8) | rx_buffer[1];
    return response & DRV8718S_DATA_MASK;
}


/**
 * Read the status register on the DRV8718s
 */
uint8_t drv8718s_read_status(void)
{
    return drv8718s_read_register(DRV8718S_REG_IC_STAT1);
}

/**
 * Read the status2 register on the DRV8718s
 */
uint8_t drv8718s_read_status2(void)
{
    return drv8718s_read_register(DRV8718S_REG_IC_STAT2);
}