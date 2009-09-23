/* 
 * linux/arch/arm/mach-at91/board-lab-at91rm9200.c
 *
 *  Copyright (C) 2009 Kuba Odias
 *
 *  SSD1906 framebuffer code:
 *     Copyright (C) 2009 Kuba Odias <kuba.odias@gmail.com>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/mtd/physmap.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/irq.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/at91rm9200_mc.h>

#include "generic.h"



static void __init lab_map_io(void)
{
	/* Initialize processor: 18.432 MHz crystal */
	at91rm9200_initialize(18432000, AT91RM9200_PQFP);

	/* DBGU on ttyS0. (Rx & Tx only) */
	at91_register_uart(0, 0, 0);

	/* USART1 on ttyS1. (Rx, Tx, CTS, RTS, DTR, DSR, DCD, RI) */
	at91_register_uart(AT91RM9200_ID_US1, 1, ATMEL_UART_CTS | ATMEL_UART_RTS
			   | ATMEL_UART_DTR | ATMEL_UART_DSR | ATMEL_UART_DCD
			   | ATMEL_UART_RI);

	/* Register USART2 at ttyS2 */
	at91_register_uart(AT91RM9200_ID_US2,2,0);

	/* set serial console to ttyS0 (ie, DBGU) */
	at91_set_serial_console(0);
}

static void __init lab_init_irq(void)
{
	at91rm9200_init_interrupts(NULL);
}

static struct at91_eth_data __initdata lab_eth_data = {
	.phy_irq_pin	= AT91_PIN_PC4,
	.is_rmii	= 0,
};

static struct at91_usbh_data __initdata lab_usbh_data = {
	.ports		= 1,
};

static struct at91_mmc_data __initdata lab_mmc_data = {
	.slot_b		= 0,
	.wire4		= 1,
	.det_pin	= AT91_PIN_PB27,
};

static struct spi_board_info lab_spi_devices[] = {
	{	/* DataFlash chip */
		.modalias	= "mtd_dataflash",
		.chip_select	= 0,
		.max_speed_hz	= 15 * 1000 * 1000,
	}
};

#if defined(CONFIG_FB_SSD1906) || defined(CONFIG_FB_SSD1906_MODULE)
#include <video/ssd1906fb.h>

/* Set PCK0 for SSD1906 */
static void lab_set_programmable_clocks(void)
{
	struct clk *pck0;
	struct clk *pllb;

	pck0 = clk_get(NULL, "pck0");
	pllb = clk_get(NULL, "pllb");

	/* PCK0 */
	at91_set_A_periph(AT91_PIN_PB27, 0);

	clk_set_parent(pck0, pllb);

	/* PCK0 rate = 24MHz */
	clk_set_rate(pck0, 24000000);

	clk_put(pck0);
	clk_put(pllb);
}

/* SSD1906 FB */
static void lab_init_video(void)
{
	/* NWAIT Signal */
	at91_set_A_periph(AT91_PIN_PC6, 0);

	/* Initialization of the Static Memory Controller for Chip Select 3 */
	at91_sys_write(AT91_SMC_CSR(3), AT91_SMC_DBW_16			/* 16 bit */
				| AT91_SMC_WSEN | AT91_SMC_NWS_(4)	/* wait states */
				| AT91_SMC_TDF_(1)			/* float time */
	);
}

/* LCD:    (active LQ043T3DX02 TFT panel), 480x272, 60Hz 
   Memory: Embedded SRAM (MCLK = BCLK = CLKI = 24.00MHz) */
static const struct ssd1906fb_regval lab_ssd1906fb_initregs[] = {
	{SSD1906REG_CMD_INIT,		0x00},	/* Command Initialization Register */
	{SSD1906REG_CLK_CONF,		0x00},	/* Memory Clock Configuration Register 
							BCLK to MCLK Frequency Ratio = 1:1 */
	/* ideal PCLK = 7.833MHz, actual PCLK value = 24MHz/3 = 8MHz, which gives 61.27Hz pixel clock rate */
	{SSD1906REG_LCD_CLK_CONF,	0x22},	/* LCD Pixel Clock Configuration Register 
							PCLK Source to PCLK Frequency Ratio = 3:1 
							PCLK Source = CLKI */
	{SSD1906REG_DISP_MODE,		0x84},	/* Display Mode Register 
							Display Blank (bit 7) = 1 
							LCD Bit-Per-Pixel Selection = 16bpp (bits [2:0] = 4) */
	{SSD1906REG_PANEL_TYPE,		0x61},	/* Panel Type Register 
							Color LCD Panel is selected
							Active Panel Data Width = 18-bit
							Panel Type = TFT */
	{SSD1906REG_MOD_RATE,		0x00},	/* MOD Rate Register 
							MOD output signal toggles every LFRAME */
	/* Horizontal Total = 520 */
	{SSD1906REG_HTOTAL,		0x40},	/* Horizontal Total Register: ((Bits[6:0])+1)*8) = Horizontal Total number of pixels 
							Horizontal Total = 520 pixels */
	/* Horizontal Display Period = 480 */
	{SSD1906REG_HDISP_PERIOD,	0x3B},	/* Horizontal Display Period Register: ((Bits[6:0])+1)*8) = Horizontal Display Period, in number of pixels 
							HDP = 480 pixels */
	/* Horizontal Display Period Start Position = 22 */
	{SSD1906REG_HDISP_PERIOD_START0,0x16},	/* Horizontal Display Period Start Position Register 0 
							HDPS bits [7:0] = 0x16 */
	{SSD1906REG_HDISP_PERIOD_START1,0x00},	/* Horizontal Display Period Start Position Register 1 
							HDPS bits [9:8] = 0x00 */
	/* Vertical Total = 286 */
	{SSD1906REG_VTOTAL0,		0x1D},	/* Vertical Total Register 0 
							Vertical Total bits [7:0] = 0x1D */
	{SSD1906REG_VTOTAL1,		0x01},	/* Vertical Total Register 1,
							Vertical Total bits [9:8] = 0x01 */
	/* Vertical Display Period = 272 */
	{SSD1906REG_VDISP_PERIOD0,	0x0F},	/* Vertical Display Period Register 0 
							VDP bits [7:0] = 0x0F */
	{SSD1906REG_VDISP_PERIOD1,	0x01},	/* Vertical Display Period Register 1 
							VDP bits [9:8] = 0x01 */
	/* Vertical Display Period Start Position = 10 */
	{SSD1906REG_VDISP_PERIOD_START0,0x0A},	/* Vertical Display Period Start Position Register 0 
							VDPS bits [7:0] = 0x0A */
	{SSD1906REG_VDISP_PERIOD_START1,0x00},	/* Vertical Display Period Start Position Register 1
							VDPS bits [9:8] = 0x00 */
	{SSD1906REG_LLINE_PWIDTH,	0x28},	/* LLINE Pulse Width Register 
							HSYNC signal is active low
							Horizontal Pulse Width = 41 clocks */
	/* LLINE Pulse Start Position = 2 */
	{SSD1906REG_LLINE_PSTART0,	0x01},	/* LLINE Pulse Start Position Register 0 */
	{SSD1906REG_LLINE_PSTART1,	0x00},	/* LLINE Pulse Start Position Register 1 */
	/* LFRAME Pulse Width should be 10 lines, but framebuffer allows max value of 8 */
	{SSD1906REG_LFRAME_PWIDTH,	0x07},	/* LFRAME Pulse Width Register 
							VSYNC signal is active low
							Vertical Pulse Width = 8 lines */
	/* LFRAME Pulse Start Position = 2 */
	{SSD1906REG_LFRAME_PSTART0,	0x01},	/* LFRAME Pulse Start Position Register 0 */
	{SSD1906REG_LFRAME_PSTART1,	0x00},	/* LFRAME Pulse Start Position Register 1 */
	{SSD1906REG_LFRAME_PSTART_OFFSET0,0x00},	/* LFRAME Pulse Start Offset Register 0 */
	{SSD1906REG_LFRAME_PSTART_OFFSET1,0x00},	/* LFRAME Pulse Start Offset Register 1 */
	{SSD1906REG_LFRAME_PSTOP_OFFSET0, 0x00},	/* LFRAME Pulse Stop Offset Register 0 */
	{SSD1906REG_LFRAME_PSTOP_OFFSET1, 0x00},	/* LFRAME Pulse Stop Offset Register 1 */
	{SSD1906REG_PS_CONF,		0x00},	/* Power Saving Configuration Register 
							Power Saving Mode is disabled */
	{SSD1906REG_PWM_CTL,		0x00},	/* PWM Clock/CV Pulse Control Register 
							LPWMOUT is forced low */
	{SSD1906REG_PWM_CONF,		0xA0},	/* PWM Clock/CV Pulse Configuration Register 
							PWMCLK Divide Select = 1024, LPWMOUT rate = ~24kHz
							PWMCLK = CLKI */
	{SSD1906REG_LPWMOUT_DUTY_CYCLE,	0x80},	/* LPWMOUT Duty Cycle Register 
							Duty cycle = 1/2 */
};

static struct ssd1906fb_pdata lab_ssd1906fb_pdata = {
	.initregs		= lab_ssd1906fb_initregs,
	.initregssize		= ARRAY_SIZE(lab_ssd1906fb_initregs),
	.platform_init_video	= lab_init_video,
};

#define LAB_FB_REG_BASE		AT91_CHIPSELECT_3
#define LAB_FB_VMEM_BASE	LAB_FB_REG_BASE + SZ_256K
#define LAB_FB_VMEM_SIZE	(SZ_256K)

static struct resource lab_ssd1906fb_resource[] = {
	[0] = {	/* video mem */
		.name   = "ssd1906 memory",
		.start  = LAB_FB_VMEM_BASE,
		.end    = LAB_FB_VMEM_BASE + LAB_FB_VMEM_SIZE -1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {	/* video registers */
		.name   = "ssd1906 registers",
		.start  = LAB_FB_REG_BASE,
		.end    = LAB_FB_REG_BASE + SZ_256 + 4*SZ_16 -1,
		.flags  = IORESOURCE_MEM,
	},
};

static u64 ssd1906fb_dmamask = DMA_BIT_MASK(32);

static struct platform_device lab_ssd1906fb_device = {
	.name		= "ssd1906fb",
	.id		= -1,
	.dev		= {
			.dma_mask		= &ssd1906fb_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &lab_ssd1906fb_pdata,
	},
	.resource	= lab_ssd1906fb_resource,
	.num_resources	= ARRAY_SIZE(lab_ssd1906fb_resource),
};

static void __init lab_add_device_video(void)
{
	platform_device_register(&lab_ssd1906fb_device);
}
#else
static void __init lab_set_programmable_clocks(void) {}
static void __init lab_add_device_video(void) {}
#endif

static void __init lab_board_init(void)
{
	/* Serial */
	at91_add_device_serial();
	/* Ethernet */
	at91_add_device_eth(&lab_eth_data);
	/* USB Host */
	at91_add_device_usbh(&lab_usbh_data);
	/* SPI */
	at91_add_device_spi(lab_spi_devices, ARRAY_SIZE(lab_spi_devices));
	/* PCK for SSD1906 */
	lab_set_programmable_clocks();
	/* VGA */
	lab_add_device_video();
}

MACHINE_START(AT91RM9200, "Linux ARM Board")
	/* Maintainer: Kuba Odias */
	.phys_io	= AT91_BASE_SYS,
	.io_pg_offst	= (AT91_VA_BASE_SYS >> 18) & 0xfffc,
	.boot_params	= AT91_SDRAM_BASE + 0x100,
	.timer		= &at91rm9200_timer,
	.map_io		= lab_map_io,
	.init_irq	= lab_init_irq,
	.init_machine	= lab_board_init,
MACHINE_END
