/* include/video/ssd1906fb.h
 *
 * (c) 2009 Kuba Odias <kuba.odias@gmail.com>
 *
 * Header file for Solomon Systech Limited SSD1906 driver code
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive for
 * more details.
 */

#ifndef	SSD1906FB_H
#define	SSD1906FB_H

#define SSD1906_LUT_SIZE		256
#define SSD1906_CHIP_REV		0x1E	/* expected chip revision number for SSD1906 */
#define SSD1906_FBID			"SSD1906"
#define SSD1906_DEVICENAME		"ssd1906fb"

/* register definitions */
#define SSD1906REG_DISP_BUF_SIZE	0x0001	/* Display Buffer Size Register */
#define SSD1906REG_CONF_READBACK	0x0002	/* Configuration Readback Register */
#define SSD1906REG_REV_CODE		0x0003	/* Product/Revision Code Register */
#define SSD1906REG_CLK_CONF		0x0004	/* Memory Clock Configuration Register */
#define SSD1906REG_LCD_CLK_CONF		0x0005	/* LCD Pixel Clock Configuration Register */
#define SSD1906REG_LUT_BLUE_DATA_WRITE	0x0008	/* Look-Up Table Blue Data Write Register */
#define SSD1906REG_LUT_GREEN_DATA_WRITE	0x0009	/* Look-Up Table Green Data Write Register */
#define SSD1906REG_LUT_RED_DATA_WRITE	0x000A	/* Look-Up Table Red Data Write Register */
#define SSD1906REG_LUT_WRITE_ADDRESS	0x000B	/* Look-Up Table Write Address Register */
#define SSD1906REG_LUT_BLUE_DATA_READ	0x000C	/* Look-Up Table Blue Data Read Register */
#define SSD1906REG_LUT_GREEN_DATA_READ	0x000D	/* Look-Up Table Green Data Read Register */
#define SSD1906REG_LUT_RED_DATA_READ	0x000E	/* Look-Up Table Red Data Read Register */
#define SSD1906REG_LUT_READ_ADDRESS	0x000F	/* Look-Up Table Read Address Register */
#define SSD1906REG_PANEL_TYPE		0x0010	/* Panel Type Register */
#define SSD1906REG_MOD_RATE		0x0011	/* MOD Rate Register */
#define SSD1906REG_HTOTAL		0x0012	/* Horizontal Total Register: ((Bits[6:0])+1)*8) = Horizontal Total number of pixels */
#define SSD1906REG_HDISP_PERIOD		0x0014	/* Horizontal Display Period Register: ((Bits[6:0])+1)*8) = Horizontal Display Period, in number of pixels */
#define SSD1906REG_HDISP_PERIOD_START0	0x0016	/* Horizontal Display Period Start Position Register 0 */
#define SSD1906REG_HDISP_PERIOD_START1	0x0017	/* Horizontal Display Period Start Position Register 1 */
#define SSD1906REG_VTOTAL0		0x0018	/* Vertical Total Register 0 */
#define SSD1906REG_VTOTAL1		0x0019	/* Vertical Total Register 1 */
#define SSD1906REG_VDISP_PERIOD0	0x001C	/* Vertical Display Period Register 0 */
#define SSD1906REG_VDISP_PERIOD1	0x001D	/* Vertical Display Period Register 1 */
#define SSD1906REG_VDISP_PERIOD_START0	0x001E	/* Vertical Display Period Start Position Register 0 */
#define SSD1906REG_VDISP_PERIOD_START1	0x001F	/* Vertical Display Period Start Position Register 1 */
#define SSD1906REG_LLINE_PWIDTH		0x0020	/* LLINE Pulse Width Register */
#define SSD1906REG_LLINE_PSTART0	0x0022	/* LLINE Pulse Start Position Register 0 */
#define SSD1906REG_LLINE_PSTART1	0x0023	/* LLINE Pulse Start Position Register 1 */
#define SSD1906REG_LFRAME_PWIDTH	0x0024	/* LFRAME Pulse Width Register */
#define SSD1906REG_LFRAME_PSTART0	0x0026	/* LFRAME Pulse Start Position Register 0 */
#define SSD1906REG_LFRAME_PSTART1	0x0027	/* LFRAME Pulse Start Position Register 1 */
#define SSD1906REG_LFRAME_PSTART_OFFSET0 0x0030	/* LFRAME Pulse Start Offset Register 0 */
#define SSD1906REG_LFRAME_PSTART_OFFSET1 0x0031	/* LFRAME Pulse Start Offset Register 1 */
#define SSD1906REG_LFRAME_PSTOP_OFFSET0	0x0034	/* LFRAME Pulse Stop Offset Register 0 */
#define SSD1906REG_LFRAME_PSTOP_OFFSET1	0x0035	/* LFRAME Pulse Stop Offset Register 1 */
#define SSD1906REG_HRTFT_SPECIAL_OUTPUT	0x0038	/* HR-TFT Special Output Register */
#define SSD1906REG_GPIO1_PSTART		0x003A	/* GPIO1 Pulse Start Register */
#define SSD1906REG_GPIO1_PSTOP		0x003B	/* GPIO1 Pulse Stop Register */
#define SSD1906REG_GPIO0_PSTART		0x003C	/* GPIO0 Pulse Start Register */
#define SSD1906REG_GPIO0_PSTOP		0x003E	/* GPIO0 Pulse Stop Register */
#define SSD1906REG_GPIO2_PDELAY		0x0040	/* GPIO2 Pulse Delay Register */
#define SSD1906REG_STN_COLOR_DEPTH_CTL	0x0045	/* STN Color Depth Control Register */
#define SSD1906REG_DITHERING_FRC_CTL	0x0050	/* Dithering/FRC Control Register */
#define SSD1906REG_DISP_MODE		0x0070	/* Display Mode Register */
#define SSD1906REG_SPECIAL_EFFECTS	0x0071	/* Special Effects Register */
#define SSD1906REG_DISP_START0		0x0074	/* Main Window Display Start Address Register 0 */
#define SSD1906REG_DISP_START1		0x0075	/* Main Window Display Start Address Register 1 */
#define SSD1906REG_DISP_START2		0x0076	/* Main Window Display Start Address Register 2 */
#define SSD1906REG_LOFFSET0		0x0078	/* Main Window Line Address Offset Register 0 */
#define SSD1906REG_LOFFSET1		0x0079	/* Main Window Line Address Offset Register 1 */
#define SSD1906REG_FLT_DISP_START0	0x007C	/* Floating Window Display Start Address Register 0 */
#define SSD1906REG_FLT_DISP_START1	0x007D	/* Floating Window Display Start Address Register 1 */
#define SSD1906REG_FLT_DISP_START2	0x007E	/* Floating Window Display Start Address Register 2 */
#define SSD1906REG_FLT_LOFFSET0		0x0080	/* Floating Window Line Address Offset Register 0 */
#define SSD1906REG_FLT_LOFFSET1		0x0081	/* Floating Window Line Address Offset Register 1 */
#define SSD1906REG_FLT_START_POSX0	0x0084	/* Floating Window Start Position X Register 0 */
#define SSD1906REG_FLT_START_POSX1	0x0085	/* Floating Window Start Position X Register 1 */
#define SSD1906REG_FLT_START_POSY0	0x0088	/* Floating Window Start Position Y Register 0 */
#define SSD1906REG_FLT_START_POSY1	0x0089	/* Floating Window Start Position Y Register 1 */
#define SSD1906REG_FLT_END_POSX0	0x008C	/* Floating Window End Position X Register 0 */
#define SSD1906REG_FLT_END_POSX1	0x008D	/* Floating Window End Position X Register 1 */
#define SSD1906REG_FLT_END_POSY0	0x0090	/* Floating Window End Position Y Register 0 */
#define SSD1906REG_FLT_END_POSY1	0x0091	/* Floating Window End Position Y Register 1 */
#define SSD1906REG_PS_CONF		0x00A0	/* Power Saving Configuration Register */
#define SSD1906REG_SW_RESET		0x00A2	/* Sofware Reset Register */
#define SSD1906REG_SCRATCH_PAD0		0x00A4	/* Scratch Pad Register 0 */
#define SSD1906REG_SCRATCH_PAD1		0x00A5	/* Scratch Pad Register 1 */
#define SSD1906REG_GPIO_CONF0		0x00A8	/* General Purpose I/O Pins Configuration Register 0 */
#define SSD1906REG_GPIO_CONF1		0x00A9	/* General Purpose I/O Pins Configuration Register 1 */
#define SSD1906REG_GPIO_CTL0		0x00AC	/* General Purpose I/O Pins Status/Control Register 0 */
#define SSD1906REG_GPIO_CTL1		0x00AD	/* General Purpose I/O Pins Status/Control Register 1 */
#define SSD1906REG_PWM_CTL		0x00B0	/* PWM Clock/CV Pulse Control Register */
#define SSD1906REG_PWM_CONF		0x00B1	/* PWM Clock/CV Pulse Configuration Register */
#define SSD1906REG_CV_PULSE_BURST_LEN	0x00B2	/* CV Pulse Burst Length Register */
#define SSD1906REG_LPWMOUT_DUTY_CYCLE	0x00B3	/* LPWMOUT Duty Cycle Register */
#define SSD1906REG_CURSOR_FEATURE	0x00C0	/* Cursor Feature Register */
#define SSD1906REG_CUR1_BLINK_TOTAL0	0x00C4	/* Cursor1 Blink Total Register 0 */
#define SSD1906REG_CUR1_BLINK_TOTAL1	0x00C5	/* Cursor1 Blink Total Register 1 */
#define SSD1906REG_CUR1_BLINK_ON0	0x00C8	/* Cursor1 Blink On Register 0 */
#define SSD1906REG_CUR1_BLINK_ON1	0x00C9	/* Cursor1 Blink On Register 1 */
#define SSD1906REG_CUR1_MEM_START0	0x00CC	/* Cursor1 Memory Start Register 0 */
#define SSD1906REG_CUR1_MEM_START1	0x00CD	/* Cursor1 Memory Start Register 1 */
#define SSD1906REG_CUR1_MEM_START2	0x00CE	/* Cursor1 Memory Start Register 2 */
#define SSD1906REG_CUR1_POSX0		0x00D0	/* Cursor1 Position X Register 0 */
#define SSD1906REG_CUR1_POSX1		0x00D1	/* Cursor1 Position X Register 1 */
#define SSD1906REG_CUR1_POSY0		0x00D4	/* Cursor1 Position Y Register 0 */
#define SSD1906REG_CUR1_POSY1		0x00D5	/* Cursor1 Position Y Register 1 */
#define SSD1906REG_CUR1_HSIZE0		0x00D8	/* Cursor1 Horizontal Size Register 0 */
#define SSD1906REG_CUR1_HSIZE1		0x00D9	/* Cursor1 Horizontal Size Register 1 */
#define SSD1906REG_CUR1_VSIZE0		0x00DC	/* Cursor1 Vertical Size Register 0 */
#define SSD1906REG_CUR1_VSIZE1		0x00DD	/* Cursor1 Vertical Size Register 1 */
#define SSD1906REG_CUR1_COLOR_INDEX1_0	0x00E0	/* Cursor1 Color Index 1 Register 0 */
#define SSD1906REG_CUR1_COLOR_INDEX1_1	0x00E1	/* Cursor1 Color Index 1 Register 1 */
#define SSD1906REG_CUR1_COLOR_INDEX2_0	0x00E4	/* Cursor1 Color Index 2 Register 0 */
#define SSD1906REG_CUR1_COLOR_INDEX2_1	0x00E5	/* Cursor1 Color Index 2 Register 1 */
#define SSD1906REG_CUR1_COLOR_INDEX3_0	0x00E8	/* Cursor1 Color Index 3 Register 0 */
#define SSD1906REG_CUR1_COLOR_INDEX3_1	0x00E9	/* Cursor1 Color Index 3 Register 1 */
#define SSD1906REG_CUR2_BLINK_TOTAL0	0x00EC	/* Cursor2 Blink Total Register 0 */
#define SSD1906REG_CUR2_BLINK_TOTAL1	0x00ED	/* Cursor2 Blink Total Register 1 */
#define SSD1906REG_CUR2_BLINK_ON0	0x00F0	/* Cursor2 Blink On Register 0 */
#define SSD1906REG_CUR2_BLINK_ON1	0x00F1	/* Cursor2 Blink On Register 1 */
#define SSD1906REG_CUR2_MEM_START0	0x00F4	/* Cursor2 Memory Start Register 0 */
#define SSD1906REG_CUR2_MEM_START1	0x00F5	/* Cursor2 Memory Start Register 1 */
#define SSD1906REG_CUR2_MEM_START2	0x00F6	/* Cursor2 Memory Start Register 2 */
#define SSD1906REG_CUR2_POSX0		0x00F8	/* Cursor2 Position X Register 0 */
#define SSD1906REG_CUR2_POSX1		0x00F9	/* Cursor2 Position X Register 1 */
#define SSD1906REG_CUR2_POSY0		0x00FC	/* Cursor2 Position Y Register 0 */
#define SSD1906REG_CUR2_POSY1		0x00FD	/* Cursor2 Position Y Register 1 */
#define SSD1906REG_CUR2_HSIZE0		0x0100	/* Cursor2 Horizontal Size Register 0 */
#define SSD1906REG_CUR2_HSIZE1		0x0101	/* Cursor2 Horizontal Size Register 1 */
#define SSD1906REG_CUR2_VSIZE0		0x0104	/* Cursor2 Vertical Size Register 0 */
#define SSD1906REG_CUR2_VSIZE1		0x0105	/* Cursor2 Vertical Size Register 1 */
#define SSD1906REG_CUR2_COLOR_INDEX1_0	0x0108	/* Cursor2 Color Index 1 Register 0 */
#define SSD1906REG_CUR2_COLOR_INDEX1_1	0x0109	/* Cursor2 Color Index 1 Register 1 */
#define SSD1906REG_CUR2_COLOR_INDEX2_0	0x010C	/* Cursor2 Color Index 2 Register 0 */
#define SSD1906REG_CUR2_COLOR_INDEX2_1	0x010D	/* Cursor2 Color Index 2 Register 1 */
#define SSD1906REG_CUR2_COLOR_INDEX3_0	0x0110	/* Cursor2 Color Index 3 Register 0 */
#define SSD1906REG_CUR2_COLOR_INDEX3_1	0x0111	/* Cursor2 Color Index 3 Register 1 */
#define SSD1906REG_CMD_INIT		0x0134	/* Command Initialization Register */


struct ssd1906fb_regval {
	u16	addr;
	u8	value;
};


struct ssd1906fb_par {
	void __iomem	*regs;
	unsigned char	display;

	unsigned int	pseudo_palette[16];
};

struct ssd1906fb_pdata {
	const struct ssd1906fb_regval	*initregs;
	const unsigned int		initregssize;
	void				(*platform_init_video)(void);
};

#endif

