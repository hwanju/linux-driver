/*******************************************************************************
*
*  NetFPGA-10G http://www.netfpga.org
*
*  File:
*        ael2005_conf.c
*
*  Project:
*
*
*  Author:
*        Marco Forconesi
*        Hwanju Kim
*
*  Description:
*        Write magic registers for PHYs (AEL2005) configuration.
*
*	 This code is initially developed for the Network-as-a-Service (NaaS) project.
*	 (under development in https://github.com/NetFPGA-NewNIC/linux-driver)
*        
*
*  Copyright notice:
*        Copyright (C) 2014 University of Cambridge
*
*  Licence:
*        This file is part of the NetFPGA 10G development base package.
*
*        This file is free code: you can redistribute it and/or modify it under
*        the terms of the GNU Lesser General Public License version 2.1 as
*        published by the Free Software Foundation.
*
*        This package is distributed in the hope that it will be useful, but
*        WITHOUT ANY WARRANTY; without even the implied warranty of
*        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*        Lesser General Public License for more details.
*
*        You should have received a copy of the GNU Lesser General Public
*        License along with the NetFPGA source package.  If not, see
*        http://www.gnu.org/licenses/.
*
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include "nf10.h"
#include "ael2005_conf.h"

extern int phy_conf_port_mask;

irqreturn_t mdio_access_interrupt_handler(int irq, void *dev_id)
{
	struct pci_dev *pdev = dev_id;
	struct nf10_adapter *adapter = (struct nf10_adapter *)pci_get_drvdata(pdev);

	/* signals mdio write finished */
	atomic_set(&adapter->mdio_access_rdy, 1);

	return IRQ_HANDLED;
}

int configure_ael2005_phy_chips(struct nf10_adapter *adapter)
{
	int ret = -ENODEV;
	u8 OP_CODE;
	u8 PRTAD;
	u8 DEVAD;
	u16 ADDR, WRITE_DATA;
	int size, i;
	int timeout;
	int port_num;
	u8 port_num_to_PRTAD[] = {0x02, 0x01, 0x00, 0x03};

	pr_info("PHY: AEL2005 configuration starts\n");

	/* configure PHYs one at a time:
	 * Port Address (PRTAD) identifies each of the Ethernet
	 * interfaces on the board. The closest to the PCIe
	 * fingers has a PRTAD hard-wired to 0x02 and corresponds to
	 * nf0 in SW */
	for(port_num = 0; port_num < 4; port_num++) {
		if (!(phy_conf_port_mask & (1 << port_num)))
			continue;

		PRTAD = port_num_to_PRTAD[port_num];
		DEVAD = MDIO_MMD_PMAPMD;
		timeout = 0;

		/* step 1: write reset registers */
		size = sizeof(reset) / sizeof(u16);
		for(i = 0; i < size; i += 2) {
			/* MDIO clause 45 Set Address Transaction */
			atomic_set(&adapter->mdio_access_rdy, 0);

			/* send Memory Write Request TLP */
			OP_CODE = 0x0;
			ADDR = reset[i];
			*(((u32 *)adapter->bar0) + 4) =
				(OP_CODE << 26)	|
				(PRTAD << 21)	|
				(DEVAD << 16)	|
				ADDR;
			do {
				msleep(2);
				timeout++;

				/* no response from HW */
				if (timeout > 20)
					return ret;
			} while (!atomic_read(&adapter->mdio_access_rdy));

			/* MDIO clause 45 Write Transaction */
			atomic_set(&adapter->mdio_access_rdy, 0);

			/* send Memory Write Request TLP */
			OP_CODE = 0x1;
			WRITE_DATA = reset[i+1];
			*(((u32 *)adapter->bar0) + 4) =
				(OP_CODE << 26)	|
				(PRTAD << 21)	|
				(DEVAD << 16)	|
				WRITE_DATA;
			do {
				msleep(2);
			} while (!atomic_read(&adapter->mdio_access_rdy));
		}

		/* step 2: write sr_edc or twinax_edc registers.
		 * (depending on the sfp+ modules) */
		size = sizeof(sr_edc) / sizeof(u16);
		for (i = 0; i < size; i += 2) {
			/* MDIO clause 45 Set Address Transaction */
			atomic_set(&adapter->mdio_access_rdy, 0);

			/* send Memory Write Request TLP */
			OP_CODE = 0x0;
			ADDR = sr_edc[i];
			*(((u32 *)adapter->bar0) + 4) =
				(OP_CODE << 26)	|
				(PRTAD << 21)	|
				(DEVAD << 16)	|
				ADDR;
			do {
				msleep(2);
			} while (!atomic_read(&adapter->mdio_access_rdy));

			/* MDIO clause 45 Write Transaction */
			atomic_set(&adapter->mdio_access_rdy, 0);

			/* send Memory Write Request TLP */
			OP_CODE = 0x1;
			WRITE_DATA = sr_edc[i+1];
			*(((u32 *)adapter->bar0) + 4) =
				(OP_CODE << 26)	|
				(PRTAD << 21)	|
				(DEVAD << 16)	|
				WRITE_DATA;
			do {
				msleep(2);
			} while (!atomic_read(&adapter->mdio_access_rdy));
		}

		/* step 1: write regs1 registers */
		size = sizeof(regs1) / sizeof(u16);
		for(i = 0; i < size; i += 2) {
			/* MDIO clause 45 Set Address Transaction */
			atomic_set(&adapter->mdio_access_rdy, 0);

			/* send Memory Write Request TLP */
			OP_CODE = 0x0;
			ADDR = regs1[i];
			*(((u32 *)adapter->bar0) + 4) =
				(OP_CODE << 26)	|
				(PRTAD << 21)	|
				(DEVAD << 16)	|
				ADDR;
			do {
				msleep(2);
			} while (!atomic_read(&adapter->mdio_access_rdy));

			/* MDIO clause 45 Write Transaction */
			atomic_set(&adapter->mdio_access_rdy, 0);

			/* send Memory Write Request TLP */
			OP_CODE = 0x1;
			WRITE_DATA = regs1[i+1];
			*(((u32 *)adapter->bar0) + 4) =
				(OP_CODE << 26)	|
				(PRTAD << 21)	|
				(DEVAD << 16)	|
				WRITE_DATA;
			do {
				msleep(2);
			} while (!atomic_read(&adapter->mdio_access_rdy));
		}
		pr_info("\tnf%d (PRTAD=%u) PHY configured\n", port_num, PRTAD);
	}
	pr_info("PHY: AEL2005 configuration finishes\n");

	return 0;
}
