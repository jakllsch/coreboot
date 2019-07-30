/*
 * ACPI support
 * written by Stefan Reinauer <stepan@openbios.org>
 *  (C) 2005 Stefan Reinauer
 *
 *
 *  Copyright 2005 AMD
 *  2005.9 yhlu modify that to more dynamic for AMD Opteron Based MB
 */

#include <arch/acpi.h>
#include <arch/smp/mpspec.h>
#include <device/pci.h>
#include <assert.h>
#include <cpu/amd/amdfam10_sysconf.h>
#include "mb_sysconf.h"

/* APIC */
unsigned long acpi_fill_madt(unsigned long current)
{
	struct device *dev;
	struct resource *res;
	struct mb_sysconf_t *m;
	unsigned sbdn, sbdnb;


	/* create all subtables for processors */
	current = acpi_create_madt_lapics(current);

	get_bus_conf();
	sbdn = sysconf.sbdn;
	m = sysconf.mb;
	sbdnb = m->sbdnb;

	/* Write NVIDIA MCP55 IOAPIC. */
	dev = dev_find_slot(m->bus_mcp55, PCI_DEVFN(sbdn + 0x1, 0));
	ASSERT(dev != NULL);

	res = find_resource(dev, PCI_BASE_ADDRESS_1);
	ASSERT(res != NULL);

	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current,
		m->apicid_mcp55, res->base, 0);

	pci_write_config32(dev, 0x7c, 0xc643c643);
	pci_write_config32(dev, 0x80, 0x8da01009);
	pci_write_config32(dev, 0x84, 0x200018d2);

	/* IRQ9 */
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
		current, 0, 9, 9, MP_IRQ_TRIGGER_LEVEL | MP_IRQ_POLARITY_LOW);

	/* Write NVIDIA IO55 IOAPIC. */
	dev = dev_find_slot(m->bus_mcp55b, PCI_DEVFN(sbdnb + 0x1, 0));
	ASSERT(dev != NULL);

	res = find_resource(dev, PCI_BASE_ADDRESS_1);
	ASSERT(res != NULL);

	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *)current,
		m->apicid_mcp55b, res->base, 32);

	pci_write_config32(dev, 0x7c, 0xc643c643);
	pci_write_config32(dev, 0x80, 0x8da01009);
	pci_write_config32(dev, 0x84, 0x200018d2);

	/* create all subtables for processors */
	/* acpi_create_madt_lapic_nmis returns current, not size. */
	current = acpi_create_madt_lapic_nmis(current,
			MP_IRQ_TRIGGER_EDGE | MP_IRQ_POLARITY_HIGH, 1);

	return current;
}
