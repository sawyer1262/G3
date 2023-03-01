// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// File name    : startup_demo.h
// Version      : V0.1
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef __STARTUP_DEMO_H__
#define __STARTUP_DEMO_H__

#include "libEFlash.h"
#include "eflash_drv.h"
#include "i2c_drv.h"
#include "i2c_reg.h"
#include "type.h"
#include "debug.h"
#include "cpm_drv.h"
#include "reset_drv.h"
#include "eflash_drv.h"

typedef enum
{ 
    Startup_From_ROM = 0,
    Startup_From_EFLASH,
}StartupMode;

extern void Startup_Demo(void);
extern void Demo_StartupConfig(StartupMode Mode, FunctionalState ResetEN);
extern void I2C_Config_Gpio_Reboot(I2C_PIN I2C_PINx, UINT8 GPIO_Dir);
extern UINT8 I2C_Read_Gpio_Data_Reboot(I2C_PIN I2C_PINx);

#endif /* __STARTUP_DEMO_H__ */

