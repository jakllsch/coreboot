/* SPDX-License-Identifier: GPL-2.0-only */
/* This file is part of the coreboot project. */

#include <console/console.h>
#include <cpu/x86/msr.h>
#include <cpu/intel/model_206ax/model_206ax.h>
#include <device/device.h>
#include <superio/hwm5_conf.h>
#include <superio/nuvoton/common/hwm.h>
#include <superio/nuvoton/nct6776/nct6776.h>
#include "x9scl.h"

#define HWM_INITVAL(reg, data) {(reg), (data)}
#define HWM_SETBANK(x) HWM_INITVAL(0x4e, (x))

static const uint8_t hwm_initvals[][2] = {
	HWM_SETBANK(0),
	HWM_INITVAL(0x12, 0x00), /* Reserved */
	HWM_INITVAL(0x18, 0x00), /* OVT# Config (enable SMIOVT1 OVT# output) */
	HWM_INITVAL(0x5d, 0x05), /* VBAT Monitor Control */
	HWM_INITVAL(0x66, 0x07), /* Reserved */
	HWM_INITVAL(0x40, 0x03), /* Configuration */
	HWM_INITVAL(0x4c, 0x10), /* SMI/OVT Config (disable OVT3, enable OVT2) */

#if 1
	HWM_SETBANK(7),
	HWM_INITVAL(0x01, 0x95), /* Enable PECI */
	HWM_INITVAL(0x03, 0x10), /* Enable Agent 0 */
	//HWM_INITVAL(0x09, 0x64), /* Tbase0 */
	HWM_SETBANK(0),
	HWM_INITVAL(0xae, 0x01), /* Enable PECI Agent0 */

#endif
	HWM_SETBANK(6),             /* 1350000/4486=300 RPM low limit */
	HWM_INITVAL(0x3a, 0x8c), /* SYSFANIN Count Limit */
	HWM_INITVAL(0x3b, 0x06), 
	HWM_INITVAL(0x3c, 0x8c), /* CPUFANIN */
	HWM_INITVAL(0x3d, 0x06),
	HWM_INITVAL(0x3e, 0x8c), /* AUXFANIN0 */
	HWM_INITVAL(0x3f, 0x06),
	HWM_INITVAL(0x40, 0x8c), /* AUXFANIN1 */
	HWM_INITVAL(0x41, 0x06),
	HWM_INITVAL(0x42, 0x8c), /* AUXFANIN2 */
	HWM_INITVAL(0x43, 0x06),

	HWM_SETBANK(0),		 /* voltage limits */
	HWM_INITVAL(0x2b, 0xba), /* CPUVCORE (1.488V) */
	HWM_INITVAL(0x2c, 0x4b), /*          (0.600V) */
	HWM_INITVAL(0x2d, 0xf9), /* VIN0 (13.19V) */  
	HWM_INITVAL(0x2e, 0xcb), /*      (10.75V) */
	HWM_INITVAL(0x2f, 0xe3), /* AVCC (2.96V - 3.63V) */
	HWM_INITVAL(0x30, 0xb9),
	HWM_INITVAL(0x31, 0xe3), /* 3VCC (2.96V - 3.63V) */
	HWM_INITVAL(0x32, 0xb9),
	HWM_INITVAL(0x33, 0xce), /* VIN1 DIMM (1.65V) */
	HWM_INITVAL(0x34, 0xa9), /*           (1.35V) */
	HWM_INITVAL(0x35, 0xac), /* VIN2 +5V (5.50V) */
	HWM_INITVAL(0x36, 0x8d), /*          (4.51V) */
	HWM_INITVAL(0x37, 0xbe), /* VIN3 -12 (-10.73V) */
	HWM_INITVAL(0x38, 0xb1), /*          (-13.25V) */

	HWM_SETBANK(5),		/* more limits in Value RAM */
	HWM_INITVAL(0x54, 0xe3), /* 3VSB limit (2.96V - 3.632V)*/
	HWM_INITVAL(0x55, 0xb9),
	HWM_INITVAL(0x56, 0xe3), /* VBAT limit (2.96V - 3.632V) */
	HWM_INITVAL(0x57, 0xb9),

	HWM_SETBANK(0),
	HWM_INITVAL(0x48, 0x2c), /* Serial Bus Address */
	HWM_INITVAL(0x39, 0x4b), /* SMIOVT1 High */
	HWM_INITVAL(0x3a, 0x46), /* SMIOVT1 Hyst */

	HWM_SETBANK(6),
	HWM_INITVAL(0x23, 0x02), /* SMIOVT3: CPUTIN */

	HWM_SETBANK(0),
	HWM_INITVAL(0xb4, 0x02), /* beep for ovt */
	HWM_INITVAL(0xb5, 0x01), /* beep en */
	HWM_INITVAL(0x04, 0x00), /* SYSFANOUT: PWM */

	HWM_SETBANK(2),
	HWM_INITVAL(0x00, 0x8c), /* CPUFAN monitor temp source (PECI Agent 0) */
	HWM_INITVAL(0x05, 0x33), /* CPUFANOUT Stop Value */
	HWM_INITVAL(0x03, 0x01), /* CPUFANOUT Step Up Time */
	HWM_INITVAL(0x04, 0x01), /* CPUFANOUT Step Down Time */
	HWM_INITVAL(0x21, 0x32), /* CPUFAN (SMART FANTM IV) Temperature 1 Register(T1) */
	HWM_INITVAL(0x22, 0x5a), /* CPUFAN (SMART FANTM IV) Temperature 2 Register(T2) */
	HWM_INITVAL(0x23, 0x5a), /* T3 */
	HWM_INITVAL(0x24, 0x5a), /* T4 */
	HWM_INITVAL(0x35, 0x5f), /* CPUFAN (SMART FANTM IV) Critical Temperature */
	HWM_INITVAL(0x38, 0x03), /* CPUFANOUT Critical Temperature Tolerance */
	HWM_INITVAL(0x27, 0x33), /* CPUFAN (SMART FANTM IV) DC or DUTY_SMF4 PWM1 */
	HWM_INITVAL(0x28, 0xff), /* PWM2 */
	HWM_INITVAL(0x29, 0xff), /* PWM3 */
	HWM_INITVAL(0x2a, 0xff), /* CPUFAN (SMART FANTM IV) DC or DUTY_SMF4 PWM4 */
	HWM_INITVAL(0x20, 0x01), /* SMART FAN IV CPUFANOUT STEP */
	HWM_INITVAL(0x02, 0x45), /* Smart Fan IV, tolerance */

	HWM_SETBANK(0),
	HWM_INITVAL(0x02, 0x03), /* CPUFAN PWM freq */

	HWM_SETBANK(1),          /* SYSFAN */ 
	HWM_INITVAL(0x00, 0x81), /* SYSFAN monitor temp source (SYSTIN) */
	HWM_INITVAL(0x05, 0x33), /* Stop Value */
	HWM_INITVAL(0x03, 0x01), /* Step Up */
	HWM_INITVAL(0x04, 0x01), /* Step Down */
	HWM_INITVAL(0x21, 0x28), /* T1 */
	HWM_INITVAL(0x22, 0x50), /* T2 */
	HWM_INITVAL(0x23, 0x50), /* T3 */
	HWM_INITVAL(0x24, 0x50), /* T4 */
	HWM_INITVAL(0x35, 0x55), /* Critical Temp */
	HWM_INITVAL(0x38, 0x05), /* Critical Temp Tolerance */
	HWM_INITVAL(0x27, 0x33), /* PWM1 */
	HWM_INITVAL(0x28, 0xff), /* PWM2 */
	HWM_INITVAL(0x29, 0xff), /* PWM3 */
	HWM_INITVAL(0x2a, 0xff), /* PWM4 */
	HWM_INITVAL(0x20, 0x01), /* STEP */
	HWM_INITVAL(0x02, 0x45), /* Smart Fan IV, tolerance */

	HWM_SETBANK(0),
	HWM_INITVAL(0x00, 0x03), /* SYSFAN PWM freq */

	HWM_SETBANK(1),
	HWM_INITVAL(0x55, 0x5f), /* SMIOVT2 Temperature Source Over-temperature (High Byte) */
	HWM_INITVAL(0x53, 0x5c), /* SMIOVT2 Temperature Source Hysteresis (High Byte) */

	HWM_SETBANK(0),
	HWM_INITVAL(0x5d, 0x00), /* VBAT Monitor Control */
	HWM_INITVAL(0x5e, 0x00), /* Current Mode Enable */ 
};

static void hwm_init(void)
{
	msr_t msr;

	msr = rdmsr(MSR_TEMPERATURE_TARGET);
	nuvoton_hwm_select_bank(X9SCL_NCT6776_HWM_BASE, 7);
	pnp_write_hwm5_index(X9SCL_NCT6776_HWM_BASE, 0x09, (msr.lo >> 16) & 0xff);

	for (size_t i = 0; i < ARRAY_SIZE(hwm_initvals); i++)
		pnp_write_hwm5_index(X9SCL_NCT6776_HWM_BASE, hwm_initvals[i][0], hwm_initvals[i][1]);
}

static void mainboard_enable(struct device *dev)
{
	hwm_init();
}

struct chip_operations mainboard_ops = {
	.enable_dev = mainboard_enable,
};
