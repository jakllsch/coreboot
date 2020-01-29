/* SPDX-License-Identifier: GPL-2.0-or-later */

#define SUPERIO_DEV		SIO0
#define SUPERIO_PNP_BASE	0x2e
#undef NCT6776_SHOW_PP
#define NCT6776_SHOW_SP1
#define NCT6776_SHOW_SP2
#define NCT6776_SHOW_KBC
#undef NCT6776_SHOW_GPIO
#define NCT6776_SHOW_HWM

#include <superio/nuvoton/nct6776/acpi/superio.asl>

#undef SUPERIO_DEV
#undef SUPERIO_PNP_BASE
#define SUPERIO_DEV		BMC0
#define SUPERIO_PNP_BASE	0x164e

#undef WPCM450_SHOW_KBC
#define WPCM450_SHOW_SP2
#undef WPCM450_SHOW_SP1
#define WPCM450_SHOW_KCS
#include <superio/nuvoton/wpcm450/acpi/superio.asl>
