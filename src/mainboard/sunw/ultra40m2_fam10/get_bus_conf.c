/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007 AMD
 * Written by Yinghai Lu <yinghailu@amd.com> for AMD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <console/console.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <string.h>
#include <stdint.h>
#include <cpu/amd/multicore.h>

#include <cpu/amd/amdfam10_sysconf.h>

#include <stdlib.h>
#include "mb_sysconf.h"

// Global variables for MB layouts and these will be shared by irqtable mptable and acpi_tables
struct mb_sysconf_t mb_sysconf;

/* Here you only need to set value in pci1234 for HT-IO that could be
installed or not You may need to preset pci1234 for HTIO board, please
refer to src/northbridge/amd/amdfam10/get_pci1234.c for detail */
static u32 pci1234x[] = {
	0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc,
	0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc,
	0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc,
	0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc,
	0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc, 0x0000ffc,
	0x0000ffc, 0x0000ffc,
	};


/* HT Chain device num, actually it is unit id base of every ht device
in chain, assume every chain only have 4 ht device at most */

static unsigned hcdnx[] = {
	0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020,
	0x20202020, 0x20202020,
};

extern void get_pci1234(void);

static unsigned get_bus_conf_done = 0;

void get_bus_conf(void)
{
	unsigned apicid_base;
	struct mb_sysconf_t *m;

	int i;

	if(get_bus_conf_done == 1) return; //do it only once

	get_bus_conf_done = 1;

	sysconf.mb = &mb_sysconf;

	m = sysconf.mb;
	memset(m, 0, sizeof(struct mb_sysconf_t));

	sysconf.hc_possible_num = ARRAY_SIZE(pci1234x);
	for(i = 0; i < sysconf.hc_possible_num; i++) {
		sysconf.pci1234[i] = pci1234x[i];
		sysconf.hcdn[i] = hcdnx[i];
	}

	get_pci1234();

	for (unsigned int j = 0; j < sysconf.hc_possible_num; j++) {
		printk(BIOS_INFO, "%s hcdn %u %x\n", __func__, j, sysconf.hcdn[j]);
		printk(BIOS_INFO, "%s pci1234 %u %x\n", __func__, j, sysconf.pci1234[j]);
	}

	sysconf.sbdn = (sysconf.hcdn[0] & 0xff); // first byte of first chain
	m->sbdnb = (sysconf.hcdn[1] & 0xff); // first byte of second chain
	m->bus_mcp55 = (sysconf.pci1234[0] >> 12) & 0xff;
	m->bus_mcp55b = (sysconf.pci1234[1] >> 12) & 0xff;

/*I/O APICs:	APIC ID	Version	State		Address*/
	if (IS_ENABLED(CONFIG_LOGICAL_CPUS))
		apicid_base = get_apicid_base(1);
	else
		apicid_base = CONFIG_MAX_PHYSICAL_CPUS;
	m->apicid_mcp55 = apicid_base+0;
	m->apicid_mcp55b = apicid_base+1;
}
