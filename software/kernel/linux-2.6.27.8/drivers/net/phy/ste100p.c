/*
 * drivers/net/phy/ste100p.c
 *
 * Driver for STE100P PHYs
 *
 * Author: Grzegorz Rajtar mcgregor@blackmesaeast.com.pl
 *
 * Copyright (c) 2007 Black Mesa East 
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

/* Control register and bitmasks*/
#define MII_STE100P_XCR_REG 0x00
#define MII_STE100P_XCR_RESET 		1 << 15
#define MII_STE100P_XCR_LOOPBACK	1 << 14
#define MII_STE100P_XCR_SPEED		1 << 13
#define MII_STE100P_XCR_AN		1 << 12
#define MII_STE100P_XCR_PWRDN		1 << 11
#define MII_STE100P_XCR_ISOLATE		1 << 10
#define MII_STE100P_XCR_RSTRT_AN	1 << 9
#define MII_STE100P_XCR_FULL_DUP	1 << 8
#define MII_STE100P_XCR_COLLEN		1 << 7


/* Iinterrupt register and bitmasks */
#define MII_STE100P_XIE_REG	0x12
#define MII_STE100P_XIE_ANCE	1 << 6
#define MII_STE100P_XIE_RFE	1 << 5
#define MII_STE100P_XIE_LDE	1 << 4
#define MII_STE100P_XIE_ANAE	1 << 3
#define MII_STE100P_XIE_PDFE	1 << 2
#define MII_STE100P_XIE_ANPE	1 << 1
#define MII_STE100P_XIE_REFE	1
#define MII_STE100P_XIE_ALL	\
(MII_STE100P_XIE_ANCE | MII_STE100P_XIE_RFE | MII_STE100P_XIE_LDE | \
 MII_STE100P_XIE_ANAE | MII_STE100P_XIE_PDFE | MII_STE100P_XIE_ANPE | MII_STE100P_XIE_REFE)

/* Interrupt status register and bitmasks */
#define MII_STE100P_XCSIIS_REG		0x11
#define MII_STE100P_XCSIIS_SPEED	1 << 9
#define MII_STE100P_XCSIIS_DUPLEX	1 << 8
#define MII_STE100P_XCSIIS_PAUSE	1 << 7
#define MII_STE100P_XCSIIS_ANC		1 << 6
#define MII_STE100P_XCSIIS_RFD		1 << 5
#define MII_STE100P_XCSIIS_LS		1 << 4
#define MII_STE100P_XCSIIS_ANAR		1 << 3
#define MII_STE100P_XCSIIS_PDF		1 << 2
#define MII_STE100P_XCSIIS_ANPR		1 << 1
#define MII_STE100P_XCSIIS_REF		1

/* 100-TX register and bitmasks*/
#define MII_STE100P_100CTR_REG		0x13
#define MII_STE100P_100CTR_DISERR	1 << 13
#define MII_STE100P_100CTR_ANC		1 << 12
#define MII_STE100P_100CTR_ENRLB	1 << 9
#define MII_STE100P_100CTR_ENDCR	1 << 8
#define MII_STE100P_100CTR_ENRZI	1 << 7
#define MII_STE100P_100CTR_EN4B5B	1 << 6
#define MII_STE100P_100CTR_ISOTX	1 << 5
#define MII_STE100P_100CTR_CMODE_MASK	0x001C
#define MII_STE100P_100CTR_DISMLT	1 << 1
#define MII_STE100P_100CTR_DISCRM	1

/* Auto-negotiation register and bitmasks*/
#define MII_STE100P_ANA_REG         0x04
#define MII_STE100P_ANA_NXTPG          1 << 15 
#define MII_STE100P_ANA_RF             1 << 13
#define MII_STE100P_ANA_FC             1 << 10
#define MII_STE100P_ANA_T4             1 << 9
#define MII_STE100P_ANA_TXF            1 << 8
#define MII_STE100P_ANA_TXH            1 << 7
#define MII_STE100P_ANA_10F            1 << 6
#define MII_STE100P_ANA_10H            1 << 5
#define MII_STE100P_ANA_SF             0x0000
#define MII_STE100P_ANA_SF_MASK        0x000F

/* PHY chip ID regs */
#define MII_STE100P_PID1_REG        0x02
#define MII_STE100P_PID2_REG        0x03

#define MII_STE100P_PHYID_VAL    0x1C040000
#define MII_STE100P_PHYID_MASK   0xFFFF0000



MODULE_DESCRIPTION("STE100P PHY driver");
MODULE_AUTHOR("Grzegorz Rajtar <mcgregor@blackmesaeast.com.pl>");
MODULE_LICENSE("GPL");

static int ste100p_config_intr(struct phy_device *phydev)
{
	int temp;
	temp = phy_read(phydev, MII_STE100P_XIE_REG);

	if(PHY_INTERRUPT_ENABLED == phydev->interrupts )
		temp |= MII_STE100P_XIE_ALL;
	else
	{
		temp &= ~(MII_STE100P_XIE_ALL);
		//clear interrupt status register
		phy_read(phydev, MII_STE100P_XCSIIS_REG);
	}

	temp = phy_write(phydev, MII_STE100P_XIE_REG, temp);

	return temp;
}

static int ste100p_config_aneg(struct phy_device *phydev)
{
	int err;

	int temp = phy_read(phydev, MII_STE100P_XCR_REG);
	int temp2;
	
	/* Isolate the PHY */

	err = phy_write(phydev, MII_STE100P_XCR_REG, temp | MII_STE100P_XCR_ISOLATE);
	
	//read for isolate latch
	temp = phy_read(phydev, MII_STE100P_XCR_REG);

	if (err < 0)
		return err;

	/* Set the Auto_negotiation Advertisement Register */
	/* MII advertising for Next page, 100BaseTxFD and HD, 10BaseTFD and HD, IEEE 802.3 */
	err =  phy_write(phydev, MII_STE100P_ANA_REG,
	    	MII_STE100P_ANA_NXTPG | MII_STE100P_ANA_TXF | MII_STE100P_ANA_TXH |
		MII_STE100P_ANA_10F | MII_STE100P_ANA_10H | MII_STE100P_ANA_SF);

	if (err < 0)
	    return err;

	err =  phy_write(phydev, MII_STE100P_XCR_REG, 
		temp | MII_STE100P_XCR_SPEED | MII_STE100P_XCR_AN | 
		MII_STE100P_XCR_FULL_DUP);

	if (err < 0)
	    return err;

	/* Restart auto negotiation */

	err = phy_write(phydev, MII_STE100P_XCR_REG,  
	    (temp | MII_STE100P_XCR_AN | MII_STE100P_XCR_RSTRT_AN) & (~MII_STE100P_XCR_ISOLATE));

	//read for isolate latch
	phy_read(phydev, MII_STE100P_XCR_REG);

	if (err < 0)
		return err;

	/* Configure the new settings */
	err = genphy_config_aneg(phydev);

	if (err < 0)
		return err;

	return 0;
}

static int ste100p_config_init(struct phy_device *phydev)
{
	int err;
	int temp;
	int temp_xcr;	

	/* Isolate the PHY */

	temp_xcr = phy_read(phydev, MII_STE100P_XCR_REG);
	
	err = phy_write(phydev, MII_STE100P_XCR_REG, 
	    ((temp_xcr |  MII_STE100P_XCR_ISOLATE) & (~MII_STE100P_XCR_AN)));

	//read for isolate latch
	temp_xcr = phy_read(phydev, MII_STE100P_XCR_REG);
		
	err = phy_write(phydev, MII_STE100P_XCR_REG,  
	    temp_xcr &  (~MII_STE100P_XCR_ISOLATE));	

	//read for isolate latch
	phy_read(phydev, MII_STE100P_XCR_REG);
	
	if (err < 0)
	    return err;

	temp = phy_read(phydev, MII_STE100P_ANA_REG);
	    
	err = phy_write(phydev, MII_STE100P_ANA_REG, temp
	    | MII_STE100P_ANA_FC | MII_STE100P_ANA_TXF 
	    | MII_STE100P_ANA_TXH | MII_STE100P_ANA_10F | MII_STE100P_ANA_10H);
	    
	if (err < 0)
	    return err;

	/* Reconnect the PHY, and enable Autonegotiation */
	err = phy_write(phydev,  MII_STE100P_XCR_REG, (temp_xcr | MII_STE100P_XCR_AN 
		| MII_STE100P_XCR_RSTRT_AN) & (~MII_STE100P_XCR_ISOLATE));

	//read for isolate latch
	phy_read(phydev, MII_STE100P_XCR_REG);

	if (err < 0)
		return err;

	return 0;
}

static int ste100p_ack_interrupt(struct phy_device *phydev)
{
	int intmask = 1;
	int rep = 0;

	//clear multiple interrupts;
	do
	{
	    intmask = phy_read(phydev, MII_STE100P_XCSIIS_REG);
	    rep++;	    	    
	} while ((intmask & MII_STE100P_XIE_ALL) != 0 && rep < 25);
	
	return  0;
}


static int ste100p_suspend(struct phy_device *phydev)
{
	int temp = phy_read(phydev, MII_STE100P_XCR_REG);
	temp = phy_write(phydev, MII_STE100P_XCR_REG, temp | MII_STE100P_XCR_PWRDN);
	//read for latch XCR REG
	phy_read(phydev, MII_STE100P_XCR_REG);
	return temp;
}

static int ste100p_resume(struct phy_device *phydev)
{
	int temp = 
	temp = phy_write(phydev, MII_STE100P_XCR_REG, temp & (~MII_STE100P_XCR_PWRDN));
	//read for latch XCR REG	
	phy_read(phydev, MII_STE100P_XCR_REG);
	return temp;
}

static struct phy_driver ste100p_driver = {
	.phy_id		= MII_STE100P_PHYID_VAL,
	.name		= "STE100P",
	.phy_id_mask	= MII_STE100P_PHYID_MASK,
	.features	= PHY_BASIC_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= &ste100p_config_init,
	.config_intr	= &ste100p_config_intr,
	.config_aneg	= &ste100p_config_aneg,
	.suspend	= &ste100p_suspend,
	.resume		= &ste100p_resume,
	.ack_interrupt 	= &ste100p_ack_interrupt,
	.read_status	= &genphy_read_status,
	.driver 	= { .owner = THIS_MODULE,},
};

static int __init ste100p_init(void)
{
	return phy_driver_register(&ste100p_driver);
}

static void __exit ste100p_exit(void)
{
	phy_driver_unregister(&ste100p_driver);
}

module_init(ste100p_init);
module_exit(ste100p_exit);
