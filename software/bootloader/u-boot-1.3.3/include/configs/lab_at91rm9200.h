/*
 * Rick Bronson <rick@efn.org>
 * Kuba Odias <kuba.odias@gmail.com>
 *
 * Configuation settings for the LAB AT91RM9200 board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/* ARM asynchronous clock */
#define AT91C_MAIN_CLOCK	179712000	/* from 18.432 MHz crystal (18432000 / 4 * 39) */
#define AT91C_MASTER_CLOCK	89856000	/* peripheral clock (AT91C_MASTER_CLOCK / 2) */

#define AT91_SLOW_CLOCK		32768	/* slow clock */

#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define CONFIG_AT91RM9200	1	/* It's an Atmel AT91RM9200 SoC	*/
#define CONFIG_AT91RM9200DK	1	/* on an AT91RM9200DK Board	*/
#undef  CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/
#define USE_920T_MMU		1

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_BAUDRATE 115200

/*
 * Hardware drivers
 */

/* define one of these to choose the DBGU, USART0  or USART1 as console */
#define CONFIG_DBGU
#undef CONFIG_USART0
#undef CONFIG_USART1

#undef	CONFIG_HWFLOW			/* don't include RTS/CTS flow control support	*/

#undef	CONFIG_MODEM_SUPPORT		/* disable modem initialization stuff */

#define CONFIG_BOOTDELAY      3
/* #define CONFIG_ENV_OVERWRITE	1 */


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MMC
// #define CONFIG_CMD_EXT2
#define CONFIG_CMD_PING

#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_AUTOSCRIPT
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MISC
#undef CONFIG_CMD_LOADS

#define SECTORSIZE 512

#define ADDR_COLUMN 1
#define ADDR_PAGE 2
#define ADDR_COLUMN_PAGE 3

#define CONFIG_NR_DRAM_BANKS 4
#define PHYS_SDRAM 0x20000000
#define PHYS_SDRAM_SIZE 0x4000000  /* 64 megabytes */

#define CFG_MEMTEST_START		PHYS_SDRAM
#define CFG_MEMTEST_END			CFG_MEMTEST_START + PHYS_SDRAM_SIZE - 262144

// TODO: remove
#define DEBUG_ETHER
#define CONFIG_DRIVER_ETHER
#define CONFIG_NET_RETRY_COUNT		20
#undef CONFIG_AT91C_USE_RMII

#define CONFIG_MISC_INIT_R

/* AC Characteristics */
/* DLYBS = tCSS = 250ns min and DLYBCT = tCSH = 250ns */
#define DATAFLASH_TCSS	(0xC << 16)
#define DATAFLASH_TCHS	(0x1 << 24)

#define CONFIG_MMC
#define CONFIG_HAS_DATAFLASH		1

#define CONFIG_NEW_PARTITION 		1
#define	CONFIG_NEW_DF_PARTITION 	1
#define CONFIG_DOS_PARTITION        	1
#define CONFIG_ISO_PARTITION        	1


#define CFG_SPI_WRITE_TOUT		(5*CFG_HZ)
#define CFG_MAX_DATAFLASH_BANKS		1
#define CFG_MAX_DATAFLASH_PAGES		16384
#define CFG_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* Logical adress for CS0 */
#define CFG_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000	/* Logical adress for CS3 */

#define PHYS_FLASH_1			0x10000000
#define PHYS_FLASH_SIZE			0x200000  /* 2 megs main flash */
#define CFG_FLASH_BASE			PHYS_FLASH_1
#define CFG_MAX_FLASH_BANKS		1
#define CFG_MAX_FLASH_SECT		256
#define CFG_FLASH_ERASE_TOUT		(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT		(2*CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_DATAFLASH

#ifdef CFG_ENV_IS_IN_DATAFLASH
#define CFG_ENV_OFFSET			0x4000
#define CFG_ENV_ADDR			(CFG_DATAFLASH_LOGIC_ADDR_CS0 + CFG_ENV_OFFSET)
#define CFG_ENV_SIZE			0x4000
#endif	/* CFG_ENV_IS_IN_DATAFLASH */


#define CFG_LOAD_ADDR		0x21000000  /* default load address */

#ifdef CONFIG_SKIP_LOWLEVEL_INIT
#define CFG_BOOT_SIZE		0x6000 /* 24 KBytes */
#define CFG_U_BOOT_BASE		(PHYS_FLASH_1 + 0x10000)
#define CFG_U_BOOT_SIZE		0x10000 /* 64 KBytes */
#else
#define CFG_BOOT_SIZE		0x00 /* 0 KBytes */
#define CFG_U_BOOT_BASE		PHYS_FLASH_1
#define CFG_U_BOOT_SIZE		0x60000 /* 384 KBytes */
#endif	/* CONFIG_SKIP_LOWLEVEL_INIT */

#define CFG_BAUDRATE_TABLE	{ 115200, 19200, 38400, 57600, 9600 }

#define CFG_PROMPT		"U-Boot> "	/* Monitor Command Prompt */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */

#define CFG_HZ 1000
#define CFG_HZ_CLOCK AT91C_MASTER_CLOCK/2	/* AT91C_TC0_CMR is implicitly set to */
						/* AT91C_TC_TIMER_DIV1_CLOCK */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif
