/* SPDX-License-Identifier: GPL-2.0-only */
/* This file is part of the coreboot project. */

/*
 * Include this file into a mainboard's DSDT _SB device tree and it will
 * expose the WPCM450 BMC/SuperIO and some of its functionality.
 *
 * It allows the change of IO ports, IRQs and DMA settings on logical
 * devices, disabling and reenabling logical devices.
 *
 *   LDN		State
 * 0x2 SP2		Implemented, untested
 * 0x3 SP1		Implemented, untested
 * 0x6 KBC		Unimplemented
 * 0x11 KCS		Unimplemented
 *
 * Controllable through preprocessor defines:
 * SUPERIO_DEV		Device identifier for this SIO (e.g. SIO0)
 * SUPERIO_PNP_BASE	I/O address of the first PnP configuration register
 * WPCM450_SHOW_SP1	If defined, Serial Port 1 will be exposed.
 * WPCM450_SHOW_SP2	If defined, Serial Port 2 will be exposed.
 */

#undef SUPERIO_CHIP_NAME
#define SUPERIO_CHIP_NAME WPCM450
#include <superio/acpi/pnp.asl>

#undef PNP_DEFAULT_PSC
#define PNP_DEFAULT_PSC Return (0) /* no power management */

Device(SUPERIO_DEV) {
	Name (_HID, EisaId("PNP0A05"))
	Name (_STR, Unicode("Nuvoton WPCM450 Super I/O"))
	Name (_UID, SUPERIO_UID(SUPERIO_DEV,))

	/* SuperIO configuration ports */
	OperationRegion (CREG, SystemIO, SUPERIO_PNP_BASE, 0x02)
	Field (CREG, ByteAcc, NoLock, Preserve)
	{
		PNP_ADDR_REG,	8,
		PNP_DATA_REG,	8,
	}
	IndexField (PNP_ADDR_REG, PNP_DATA_REG, ByteAcc, NoLock, Preserve)
	{
		Offset (0x07),
		PNP_LOGICAL_DEVICE,	8, /* Logical device selector */

		Offset (0x30),
		PNP_DEVICE_ACTIVE,	1, /* Logical device activation */
		ACT1,			1, /* Logical device activation */
		ACT2,			1, /* Logical device activation */
		ACT3,			1, /* Logical device activation */
		ACT4,			1, /* Logical device activation */
		ACT5,			1, /* Logical device activation */
		ACT6,			1, /* Logical device activation */
		ACT7,			1, /* Logical device activation */

		Offset (0x60),
		PNP_IO0_HIGH_BYTE,	8, /* First I/O port base - high byte */
		PNP_IO0_LOW_BYTE,	8, /* First I/O port base - low byte */
		Offset (0x62),
		PNP_IO1_HIGH_BYTE,	8, /* Second I/O port base - high byte */
		PNP_IO1_LOW_BYTE,	8, /* Second I/O port base - low byte */
		Offset (0x64),
		PNP_IO2_HIGH_BYTE,	8, /* Third I/O port base - high byte */
		PNP_IO2_LOW_BYTE,	8, /* Third I/O port base - low byte */

		Offset (0x70),
		PNP_IRQ0,		8, /* First IRQ */
		Offset (0x72),
		PNP_IRQ1,		8, /* Second IRQ */
		Offset (0x74),
		PNP_DMA0,		8, /* DRQ */
	}

	Method (_CRS)
	{
		/* Announce the used I/O ports to the OS */
		Return (ResourceTemplate () {
			IO (Decode16, SUPERIO_PNP_BASE, SUPERIO_PNP_BASE, 0x01, 0x02)
		})
	}

	#undef PNP_ENTER_MAGIC_1ST
	#undef PNP_ENTER_MAGIC_2ND
	#undef PNP_ENTER_MAGIC_3RD
	#undef PNP_ENTER_MAGIC_4TH
	#undef PNP_EXIT_MAGIC_1ST
	#undef PNP_EXIT_SPECIAL_REG
	#undef PNP_EXIT_SPECIAL_VAL
	#include <superio/acpi/pnp_config.asl>

#ifdef WPCM450_SHOW_SP1
	#undef SUPERIO_UART_LDN
	#undef SUPERIO_UART_DDN
	#undef SUPERIO_UART_PM_REG
	#undef SUPERIO_UART_PM_VAL
	#undef SUPERIO_UART_PM_LDN
	#define SUPERIO_UART_LDN 3
	#include <superio/acpi/pnp_uart.asl>
#endif

#ifdef WPCM450_SHOW_SP2
	#undef SUPERIO_UART_LDN
	#undef SUPERIO_UART_DDN
	#undef SUPERIO_UART_PM_REG
	#undef SUPERIO_UART_PM_VAL
	#undef SUPERIO_UART_PM_LDN
	#define SUPERIO_UART_LDN 2
	#include <superio/acpi/pnp_uart.asl>
#endif
}
