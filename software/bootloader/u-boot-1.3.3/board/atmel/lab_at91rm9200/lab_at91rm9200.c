/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Kuba Odias <kuba.odias@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/AT91RM9200.h>
#include <at91rm9200_net.h>
#include <ste100p.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* Enable Ctrlc */
	console_init_f ();

	/* memory and cpu-speed are setup before relocation */
	/* so we do _nothing_ here */

	/* arch number of AT91RM9200DK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91RM9200;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_DRIVER_ETHER
#if defined(CONFIG_CMD_NET)

/*
 * Name:
 *	at91rm9200_GetPhyInterface
 * Description:
 *	Initialise the interface functions to the PHY
 * Arguments:
 *	None
 * Return value:
 *	None
 */
void at91rm9200_GetPhyInterface(AT91PS_PhyOps p_phyops)
{
	p_phyops->Init = ste100p_InitPhy;
	p_phyops->IsPhyConnected = ste100p_IsPhyConnected;
	p_phyops->GetLinkSpeed = ste100p_GetLinkSpeed;
	p_phyops->AutoNegotiate = ste100p_AutoNegotiate;
}

#endif
#endif	/* CONFIG_DRIVER_ETHER */

/* Configure MAC address and init STE100P */
#if defined(CONFIG_CMD_NET)
#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	int i;
	char *tmp,*end;
	tmp = getenv("ethaddr");
	if (tmp==NULL)
	{
		printf("MAC address not set, networking is not operational\n");
		return 0;
	}
	bd_t bd;
	for (i=0; i<6; i++)
	{
		bd.bi_enetaddr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
		if (tmp)
			tmp = (*end) ? end+1 : end;
	}
	printf("Using MAC address %02X:%02X:%02X:%02X:%02X:%02X\n",
	bd.bi_enetaddr[0],bd.bi_enetaddr[1],bd.bi_enetaddr[2],
	bd.bi_enetaddr[3],bd.bi_enetaddr[4],bd.bi_enetaddr[5]);

	return eth_init(&bd);
}
#endif
#endif

