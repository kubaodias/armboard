/*
 * Copyright (C) 2004-2006 Atmel Corporation
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
#ifndef __ASM_AT91RM9200_MMC_H
#define __ASM_AT91RM9200_MMC_H


struct mmc_cid {
	unsigned long psn;
	unsigned short oid;
	unsigned char mid;
	unsigned char prv;
	unsigned char mdt;
	char pnm[7];
};

struct mmc_csd
{
	u8	csd_structure;
	u8	spec_vers;
	u8	taac;
	u8	nsac;
	u8	tran_speed;
	u16	ccc;
	u8 read_bl_len;
	u8 read_bl_partial;
    u8 write_blk_misalign;
    u8 read_blk_misalign;
	u8 dsr_imp;
	u32 c_size;
	u8 vdd_r_curr_min;      //Not exist in v2.0
	u8 vdd_r_curr_max;      //Not exist in v2.0
	u8 vdd_w_curr_min;      //Not exist in v2.0
	u8 vdd_w_curr_max;      //Not exist in v2.0
	u8 c_size_mult;         //Not exist in v2.0
	u8 sector_size;
	u8 wp_grp_size;
	u8 wp_grp_enable;
	u8 default_ecc;
	u8 r2w_factor;
	u8 write_bl_len;
	u8 write_bl_partial;
	u8 copy;
	u8 perm_write_protect;
	u8 tmp_write_protect;
	u8 file_format;
    u8 file_format_grp;
	u8 ecc;
	u8	crc;
};

/* MMC Command numbers */
#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID 		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SD_SEND_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_CSD 		9
#define MMC_CMD_SEND_CID 		10
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_WRITE_BLOCK		24
#define MMC_CMD_APP_CMD			55

#define MMC_ACMD_SD_SEND_OP_COND	41
#define MMC_CMD_SD_SEND_IF_COND 8
#define MMC_CMD_SD_READ_OCR 58

#define R1_ILLEGAL_COMMAND		(1 << 22)
#define R1_APP_CMD			(1 << 5)

#define MMC_ACMD_SD_SEND_IF_COND 8

#endif /* __ASM_AT91RM9200_MMC_H */

