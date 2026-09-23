#ifndef DRV8718S_INTERFACE_H
#define DRV8718S_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>
#include "cy_pdl.h"

/**
 *  Interface code for DRV8718s motor driver
 *    Initializing, configuring registers, 
 *    enabling/disabling driver, etc.
 */

/////////////// STARTUP //////////////////////
bool drv8718s_init(void);

/////////////// RUNNING //////////////////////
void drv8718s_enable(void);
void drv8718s_disable(void);
void drv8718s_reset(void);
void drv8718s_clear_faults(void);
bool drv8718s_check_status(void);
void drv8718s_print_status(void);

///////////// REGISTER INFORMATION ///////////
/* ============= DRV8718S SPI Protocol ============= */
/* Based on Table 7-18: SDI Input Data Word Format */

/* SPI frame structure (16 bits):
   Bit 15:    0 (always 0)
   Bit 14:    W0 (0=write, 1=read)
   Bits 13-8: Address (A5-A0, 6 bits)
   Bits 7-0:  Data (D7-D0, 8 bits)
*/

#define DRV8718S_SPI_READ_BIT       0x4000  /* Bit 14 = 1 for READ */
#define DRV8718S_SPI_WRITE_BIT      0x0000  /* Bit 14 = 0 for WRITE */
#define DRV8718S_ADDR_SHIFT         8       /* Address bits 13-8 */
#define DRV8718S_DATA_MASK          0xFF    /* Data bits 7-0 */

/* ============= Pin 9 Mode Selection ============= */
#define DRV_PIN9_MODE_FAULT_OUTPUT  1
#define DRV_PIN9_MODE               DRV_PIN9_MODE_FAULT_OUTPUT

/* ============= DRV8718S-Q1 Complete Register Map ============= */
/* Based on Table 8-1, DRV8718-Q1 Datasheet SLVSEA2D            */

/* ---- Status Registers (Read Only) ---- */
#define DRV8718S_REG_IC_STAT1       0x00    /* IC Status: SPI_OK, POR, FAULT, WARN, DS_GS, UV, OV, OT_WD_AGD */
#define DRV8718S_REG_VDS_STAT1      0x01    /* VDS Status HB1-4 */
#define DRV8718S_REG_VDS_STAT2      0x02    /* VDS Status HB5-8 */
#define DRV8718S_REG_VGS_STAT1      0x03    /* VGS Status HB1-4 */
#define DRV8718S_REG_VGS_STAT2      0x04    /* VGS Status HB5-8 */
#define DRV8718S_REG_IC_STAT2       0x05    /* PVDD_UV, PVDD_OV, VCP_UV, OTW, OTSD, WD_FLT, SCLK_FLT */
#define DRV8718S_REG_IC_STAT3       0x06    /* IC_ID */

/* ---- IC Control Registers ---- */
#define DRV8718S_REG_IC_CTRL1       0x07    /* EN_DRV, EN_OLSC, LOCK, CLR_FLT */
#define DRV8718S_REG_IC_CTRL2       0x08    /* DIS_SSC, DRVOFF_nFLT, CP_MODE, WD_EN, WD_FLT_M, WD_WIN, WD_RST */

/* ---- Bridge Control Registers ---- */
#define DRV8718S_REG_BRG_CTRL1      0x09    /* HB1-4 output state control */
#define DRV8718S_REG_BRG_CTRL2      0x0A    /* HB5-8 output state control */

/* ---- PWM Control Registers ---- */
#define DRV8718S_REG_PWM_CTRL1      0x0B    /* HB1-4 PWM mapping */
#define DRV8718S_REG_PWM_CTRL2      0x0C    /* HB5-8 PWM mapping */
#define DRV8718S_REG_PWM_CTRL3      0x0D    /* HB1-8 high-side/low-side drive (HL) */
#define DRV8718S_REG_PWM_CTRL4      0x0E    /* HB1-8 freewheeling config (FW) */

/* ---- Gate Drive Current Registers ---- */
#define DRV8718S_REG_IDRV_CTRL1     0x0F    /* HB1 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL2     0x10    /* HB2 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL3     0x11    /* HB3 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL4     0x12    /* HB4 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL5     0x13    /* HB5 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL6     0x14    /* HB6 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL7     0x15    /* HB7 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL8     0x16    /* HB8 gate drive source/sink */
#define DRV8718S_REG_IDRV_CTRL9     0x17    /* HB1-8 low current control */

/* ---- Driver Control Registers ---- */
#define DRV8718S_REG_DRV_CTRL1      0x18    /* VGS_MODE, VGS_IND, VGS_LVL, VGS_HS_DIS */
#define DRV8718S_REG_DRV_CTRL2      0x19    /* VGS_TDRV HB1-4 */
#define DRV8718S_REG_DRV_CTRL3      0x1A    /* VGS_TDRV HB5-8 */
#define DRV8718S_REG_DRV_CTRL4      0x1B    /* VGS_TDEAD HB1-8 */
#define DRV8718S_REG_DRV_CTRL5      0x1C    /* VDS_DG HB1-8 */
#define DRV8718S_REG_DRV_CTRL6      0x1D    /* VDS fault pulldown current */
#define DRV8718S_REG_DRV_CTRL7      0x1E    /* Reserved */

/* ---- VDS Overcurrent Threshold ---- */
#define DRV8718S_REG_VDS_CTRL1      0x1F    /* HB1-2 VDS threshold */
#define DRV8718S_REG_VDS_CTRL2      0x20    /* HB3-4 VDS threshold */
#define DRV8718S_REG_VDS_CTRL3      0x21    /* HB5-6 VDS threshold */
#define DRV8718S_REG_VDS_CTRL4      0x22    /* HB7-8 VDS threshold */

/* ---- Offline Diagnostics ---- */
#define DRV8718S_REG_OLSC_CTRL1     0x23    /* HB1-4 offline diagnostic */
#define DRV8718S_REG_OLSC_CTRL2     0x24    /* HB5-8 offline diagnostic */

/* ---- Undervoltage/Overvoltage ---- */
#define DRV8718S_REG_UVOV_CTRL      0x25    /* UV/OV monitor config */

/* ---- Current Sense Amplifier ---- */
#define DRV8718S_REG_CSA_CTRL1      0x26    /* Shunt amp 1&2 config */
#define DRV8718S_REG_CSA_CTRL2      0x27    /* Shunt amp 1 blanking */
#define DRV8718S_REG_CSA_CTRL3      0x28    /* Shunt amp 2 blanking */

/* ----Duty Cycle Compensation ----- */
#define DRV8718S_REG_DCC_CTRL1      0x39    /* Enable duty cycle compensation */

/* ============= IC_STAT1 Register (0x00) Bit Definitions ============= */
#define IC_STAT1_OT_WD_AGD  (1 << 0)    /* OT, WD, or AGD fault */
#define IC_STAT1_OV         (1 << 1)    /* Overvoltage */
#define IC_STAT1_UV         (1 << 2)    /* Undervoltage */
#define IC_STAT1_DS_GS      (1 << 3)    /* VDS or VGS fault */
#define IC_STAT1_WARN       (1 << 4)    /* Warning */
#define IC_STAT1_FAULT      (1 << 5)    /* Fault active */
#define IC_STAT1_POR        (1 << 6)    /* Power-on reset */
#define IC_STAT1_SPI_OK     (1 << 7)    /* SPI communication OK */

/* Mask for real faults only (bits 0-3, 5) */
#define IC_STAT1_FAULT_MASK  (IC_STAT1_FAULT | \
                              IC_STAT1_UV | IC_STAT1_OV | \
                              IC_STAT1_OT_WD_AGD)
                              
/* ============= IC_CTRL1 Register (0x07) Bit Definitions ============= */
#define IC_CTRL1_EN_DRV   (1 << 7)   // ✅ correct
#define IC_CTRL1_EN_OLSC  (1 << 6)   // ✅ correct
#define IC_CTRL1_BRG_MODE     (0x00)     //
#define IC_CTRL1_LOCK     (0x5)     // bits [3:1]
#define IC_CTRL1_CLR_FLT  (1 << 0)   // ✅ correct

/* ============= IC_CTRL2 Register (0x08) Bit Definitions ============= */
#define IC_CTRL2_WD_RST     (1 << 0)    /* Watchdog reset */
#define IC_CTRL2_WD_WIN     (1 << 1)    /* Watchdog window */
#define IC_CTRL2_WD_FLT_M   (1 << 2)    /* Watchdog fault mode */
#define IC_CTRL2_WD_EN      (1 << 3)    /* Watchdog enable */
#define IC_CTRL2_CP_MODE    (0 << 4)    /* Charge pump mode */
#define IC_CTRL2_DRVOFF_NFLT (1 << 6)   /* DRVOFF/nFLT pin config */
#define IC_CTRL2_DIS_SSC    (1 << 7)    /* Disable spread spectrum */

/* ============= BRG_CTRL1 Register (0x09) Bit Definitions ============= */
/* Controls half-bridges 1-4 (Phase A and B for dual H-bridge) */
#define BRG_CTRL1_HB4_CTRL  (3 << 6)
#define BRG_CTRL1_HB3_CTRL  (3 << 4)
#define BRG_CTRL1_HB2_CTRL  (3 << 2)
#define BRG_CTRL1_HB1_CTRL  (3 << 0)

/* HB Control Values */
#define HB_CTRL_HIZ         0x00    /* Hi-Z */
#define HB_CTRL_LOW         0x01    /* Low-side on */
#define HB_CTRL_HIGH        0x02    /* High-side on */
#define HB_CTRL_PWM         0x03    /* PWM control */

/* ============= IDRV_CTRLx Register (0x0F-0x12) Bit Definitions ============= */
#define IDRV_CTRL_16MA     0b1010   /* 16mA */
#define IDRV_CTRL_31MA     0b1101   /* 31mA */
#define IDRV_CTRL_48MA     0b1110   /* 48mA */
#define IDRV_CTRL_62MA     0b1111   /* 62mA -- technically the default value */

/* ============= TDRV_CTRLx Register (0x0F-0x12) Bit Definitions ============= */
#define VGS_TDRV_2US        0b000   /* 2us */
#define VGS_TDRV_4US        0b001   /* 4us */
#define VGS_TDRV_8US        0b010   /* 8us */


#endif