/*
 * Copyright (C) 2004-2006 Atmel Corporation
 * Modification from AVR32 to AT91RM9200 by Lucjan Bryndza <lucjan.bryndza.ep.com.pl>
 * Also support Atmel hardware fix bswap in at91rm9200
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

#ifdef CONFIG_MMC

#include <part.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/errno.h>
#include <asm/byteorder.h>
#include <asm/arch/hardware.h>

#include <atmel_mci.h>

#ifdef DEBUG
#define pr_debug(fmt, args...) printf(fmt, ##args)
#else
#define pr_debug(...) do { } while(0)
#endif

#ifndef CFG_MMC_CLK_OD
#define CFG_MMC_CLK_OD		150000
#endif

#ifndef CFG_MMC_CLK_PP
#define CFG_MMC_CLK_PP		5000000
#endif

#ifndef CFG_MMC_OP_COND
#define CFG_MMC_OP_COND		0x00100000
#endif

#define MMC_DEFAULT_BLKLEN	512
#define MMC_DEFAULT_RCA		1

static unsigned int mmc_rca;
static int mmc_card_is_sd;
static int mmc_card_is_sdhc;
static block_dev_desc_t mmc_blkdev;

block_dev_desc_t *mmc_get_dev(int dev)
{
	return &mmc_blkdev;
}

static void mci_set_mode(unsigned long hz, unsigned long blklen)
{
	unsigned long bus_hz;
	unsigned long clkdiv;

	bus_hz = AT91C_MASTER_CLOCK;
	clkdiv = (bus_hz / hz) / 2 - 1;

	pr_debug("mmc: setting clock %lu Hz, block size %lu\n",
		 hz, blklen);

	if (clkdiv & ~255UL) {
		clkdiv = 255;
		printf("mmc: clock %lu too low; setting CLKDIV to 255\n",
			hz);
	}

	blklen &= 0xfffc;
	mmci_writel(MR, (MMCI_BF(CLKDIV, clkdiv)
			 | MMCI_BF(BLKLEN, blklen)));
//			 | MMCI_BIT(RDPROOF)
//			 | MMCI_BIT(WRPROOF)));
}

#define RESP_NO_CRC	1
#define R1		MMCI_BF(RSPTYP, 1)
#define R2		MMCI_BF(RSPTYP, 2)
#define R3		(R1 | RESP_NO_CRC)
#define R6		R1
#define R7      R3
#define NID		MMCI_BF(MAXLAT, 0)
#define NCR		MMCI_BF(MAXLAT, 1)
#define TRCMD_START	MMCI_BF(TRCMD, 1)
#define TRDIR_READ	MMCI_BF(TRDIR, 1)
#define TRTYP_BLOCK	MMCI_BF(TRTYP, 0)
#define INIT_CMD	MMCI_BF(SPCMD, 1)
#define OPEN_DRAIN	MMCI_BF(OPDCMD, 1)

#define ERROR_FLAGS	(MMCI_BIT(DTOE)			\
			 | MMCI_BIT(RDIRE)		\
			 | MMCI_BIT(RENDE)		\
			 | MMCI_BIT(RINDE)		\
			 | MMCI_BIT(RTOE))

static int
mmc_cmd(unsigned long cmd, unsigned long arg,
	void *resp, unsigned long flags)
{
	unsigned long *response = resp;
	int i, response_words = 0;
	unsigned long error_flags;
	u32 status;

	pr_debug("mmc: CMD%lu 0x%lx (flags 0x%lx)\n",
		 cmd, arg, flags);

	error_flags = ERROR_FLAGS;
	if (!(flags & RESP_NO_CRC))
		error_flags |= MMCI_BIT(RCRCE);

	flags &= ~MMCI_BF(CMDNB, ~0UL);

	if (MMCI_BFEXT(RSPTYP, flags) == MMCI_RSPTYP_48_BIT_RESP)
		response_words = 1;
	else if (MMCI_BFEXT(RSPTYP, flags) == MMCI_RSPTYP_136_BIT_RESP)
		response_words = 4;

	mmci_writel(ARGR, arg);
	mmci_writel(CMDR, cmd | flags);
	do {
		udelay(40);
		status = mmci_readl(SR);
	} while (!(status & MMCI_BIT(CMDRDY)));

	pr_debug("mmc: status 0x%08lx\n", status);

	if (status & ERROR_FLAGS) {
		printf("mmc: command %lu failed (status: 0x%08lx)\n",
		       cmd, status);
		return -EIO;
	}

	if (response_words)
		pr_debug("mmc: response:");

	for (i = 0; i < response_words; i++) {
		response[i] = mmci_readl(RSPR);
		pr_debug(" %08lx", response[i]);
	}
	pr_debug("\n");

	return 0;
}

static int mmc_acmd(unsigned long cmd, unsigned long arg,
		    void *resp, unsigned long flags)
{
	unsigned long aresp[4];
	int ret;

	/*
	 * Seems like the APP_CMD part of an ACMD has 64 cycles max
	 * latency even though the ACMD part doesn't. This isn't
	 * entirely clear in the SD Card spec, but some cards refuse
	 * to work if we attempt to use 5 cycles max latency here...
	 */
	ret = mmc_cmd(MMC_CMD_APP_CMD, 0, aresp,
		      R1 | NCR | (flags & OPEN_DRAIN));
	if (ret)
		return ret;
	if ((aresp[0] & (R1_ILLEGAL_COMMAND | R1_APP_CMD)) != R1_APP_CMD)
		return -ENODEV;

	ret = mmc_cmd(cmd, arg, resp, flags);
	return ret;
}

static unsigned long
mmc_bread(int dev, unsigned long start, lbaint_t blkcnt,
	  unsigned long *buffer)
{
	int ret, i = 0;
	unsigned long resp[4];
	unsigned long card_status, data;
	unsigned long wordcount;
	u32 status;

	if (blkcnt == 0)
		return 0;

	pr_debug("mmc_bread: dev %d, start %lx, blkcnt %lx\n",
		 dev, start, blkcnt);

	/* Put the device into Transfer state */
	ret = mmc_cmd(MMC_CMD_SELECT_CARD, mmc_rca << 16, resp, R1 | NCR);
	if (ret) goto fail;

	/* Set block length */
	ret = mmc_cmd(MMC_CMD_SET_BLOCKLEN, mmc_blkdev.blksz, resp, R1 | NCR);
	if (ret) goto fail;

	pr_debug("MCI_DTOR = %08lx\n", mmci_readl(DTOR));

	for (i = 0; i < blkcnt; i++, start++) {
		ret = mmc_cmd(MMC_CMD_READ_SINGLE_BLOCK,
			      (mmc_card_is_sdhc==0)?(start*mmc_blkdev.blksz):(start),
                  resp,(R1 | NCR | TRCMD_START | TRDIR_READ
			       | TRTYP_BLOCK));
		if (ret) goto fail;

		ret = -EIO;
		wordcount = 0;
		do {
			do {
				status = mmci_readl(SR);
				if (status & (ERROR_FLAGS | MMCI_BIT(OVRE)))
					goto fail;
			} while (!(status & MMCI_BIT(RXRDY)));

			if (status & MMCI_BIT(RXRDY)) {
				//AT91RM9200 Hardware bug
				data = swab32(mmci_readl(RDR));
				/* pr_debug("%x\n", data); */
				*buffer++ = data;
				wordcount++;
			}
		} while(wordcount < (mmc_blkdev.blksz / 4));

		pr_debug("mmc: read %u words, waiting for BLKE\n", wordcount);

		do {
			status = mmci_readl(SR);
		} while (!(status & MMCI_BIT(BLKE)));

		//putc('.');
	}

out:
	/* Put the device back into Standby state */
	mmc_cmd(MMC_CMD_SELECT_CARD, 0, resp, NCR);
	return i;

fail:
	mmc_cmd(MMC_CMD_SEND_STATUS, mmc_rca << 16, &card_status, R1 | NCR);
	printf("mmc: bread failed, card status = %08x\n", card_status);
	goto out;
}

static void mmc_parse_cid(struct mmc_cid *cid, unsigned long *resp)
{
	cid->mid = resp[0] >> 24;
	cid->oid = (resp[0] >> 8) & 0xffff;
	cid->pnm[0] = resp[0];
	cid->pnm[1] = resp[1] >> 24;
	cid->pnm[2] = resp[1] >> 16;
	cid->pnm[3] = resp[1] >> 8;
	cid->pnm[4] = resp[1];
	cid->pnm[5] = resp[2] >> 24;
	cid->pnm[6] = 0;
	cid->prv = resp[2] >> 16;
	cid->psn = (resp[2] << 16) | (resp[3] >> 16);
	cid->mdt = resp[3] >> 8;
}

static void sd_parse_cid(struct mmc_cid *cid, unsigned long *resp)
{
	cid->mid = resp[0] >> 24;
	cid->oid = (resp[0] >> 8) & 0xffff;
	cid->pnm[0] = resp[0];
	cid->pnm[1] = resp[1] >> 24;
	cid->pnm[2] = resp[1] >> 16;
	cid->pnm[3] = resp[1] >> 8;
	cid->pnm[4] = resp[1];
	cid->pnm[5] = 0;
	cid->pnm[6] = 0;
	cid->prv = resp[2] >> 24;
	cid->psn = (resp[2] << 8) | (resp[3] >> 24);
	cid->mdt = (resp[3] >> 8) & 0x0fff;
}



/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
static int mmc_parse_csd(struct mmc_csd *csd,unsigned long *resp)
{
    csd->csd_structure = UNSTUFF_BITS(resp,126,2);
    csd->spec_vers = UNSTUFF_BITS(resp,122,4);
    csd->taac = UNSTUFF_BITS(resp,112,8);
    csd->nsac = UNSTUFF_BITS(resp,104,8);
    csd->tran_speed = UNSTUFF_BITS(resp,96,8);
    csd->ccc = UNSTUFF_BITS(resp,84,12);
    csd->read_bl_len = UNSTUFF_BITS(resp,80,4);
    csd->read_bl_partial = UNSTUFF_BITS(resp,79,1);
    csd->write_blk_misalign = UNSTUFF_BITS(resp,78,1);
    csd->read_blk_misalign = UNSTUFF_BITS(resp,77,1);
    csd->dsr_imp = UNSTUFF_BITS(resp,76,1);
    if(csd->csd_structure==0)
    {
        //Structure version 1
        csd->c_size = UNSTUFF_BITS(resp,62,12);
        csd->vdd_r_curr_min = UNSTUFF_BITS(resp,59,3);
        csd->vdd_r_curr_max = UNSTUFF_BITS(resp,56,3);
        csd->vdd_w_curr_min = UNSTUFF_BITS(resp,53,3);
        csd->vdd_w_curr_max = UNSTUFF_BITS(resp,50,3);
        csd->c_size_mult = UNSTUFF_BITS(resp,47,3);
    }
    else
    {
        //Structure version 2
        csd->c_size = UNSTUFF_BITS(resp,48,22);
    }
    csd->sector_size = UNSTUFF_BITS(resp,39,7);
    csd->wp_grp_size = UNSTUFF_BITS(resp,32,7);
    csd->wp_grp_enable = UNSTUFF_BITS(resp,31,1);
    csd->default_ecc = UNSTUFF_BITS(resp,29,2);
    csd->r2w_factor = UNSTUFF_BITS(resp,26,3);
    csd->write_bl_len = UNSTUFF_BITS(resp,22,4);
    csd->write_bl_partial = UNSTUFF_BITS(resp,21,1);
    csd->file_format_grp = UNSTUFF_BITS(resp,15,1);
    csd->copy = UNSTUFF_BITS(resp,14,1);
    csd->perm_write_protect = UNSTUFF_BITS(resp,13,1);
    csd->tmp_write_protect = UNSTUFF_BITS(resp,12,1);
    csd->file_format = UNSTUFF_BITS(resp,10,2);
    csd->ecc = UNSTUFF_BITS(resp,8,2);
    csd->crc = UNSTUFF_BITS(resp,1,7);
    return 0;
}


static void mmc_dump_cid(const struct mmc_cid *cid)
{
	printf("Manufacturer ID:       %02lX\n", cid->mid);
	printf("OEM/Application ID:    %04lX\n", cid->oid);
	printf("Product name:          %s\n", cid->pnm);
	printf("Product Revision:      %lu.%lu\n",
	       cid->prv >> 4, cid->prv & 0x0f);
	printf("Product Serial Number: %lu\n", cid->psn);
	printf("Manufacturing Date:    %02lu/%02lu\n",
	       cid->mdt >> 4, cid->mdt & 0x0f);
}

static void mmc_dump_csd(const struct mmc_csd *csd)
{
	//unsigned long *csd_raw = (unsigned long *)csd;
	//printf("CSD data: %08lx %08lx %08lx %08lx\n",
	 //      csd_raw[0], csd_raw[1], csd_raw[2], csd_raw[3]);
	unsigned int card_capacity;
    	printf("CSD structure version:   1.%u\n", csd->csd_structure);
	printf("MMC System Spec version: %u\n", csd->spec_vers);
	printf("Card command classes:    %03x\n", csd->ccc);
	printf("Read block length:       %u\n", 1 << csd->read_bl_len);
	if (csd->read_bl_partial)
		puts("Supports partial reads\n");
	else
		puts("Does not support partial reads\n");
	printf("Write block length:      %u\n", 1 << csd->write_bl_len);
	if (csd->write_bl_partial)
		puts("Supports partial writes\n");
	else
		puts("Does not support partial writes\n");
	if (csd->wp_grp_enable)
		printf("Supports group WP:      %u\n", csd->wp_grp_size + 1);
	else
		puts("Does not support group WP\n");
	if(csd->csd_structure==0)
	{
		card_capacity = (csd->c_size + 1) * (1 << (csd->c_size_mult + 2)) *
                        (1 << csd->read_bl_len);
		card_capacity /= (1024*1024);
    	}
    	else
    	{
        	card_capacity = csd->c_size/2;
    	}
    	printf("Card capacity:		%u MB\n",card_capacity);
	printf("File format:            %u/%u\n",
	       csd->file_format_grp, csd->file_format);
	puts("Write protection:        ");
	if (csd->perm_write_protect)
		puts(" permanent");
	if (csd->tmp_write_protect)
		puts(" temporary");
	putc('\n');
}

static int mmc_idle_cards(void)
{
	int ret;

	/* Reset and initialize all cards */
	ret = mmc_cmd(MMC_CMD_GO_IDLE_STATE, 0, NULL, 0);
	if (ret)
		return ret;

	/* Keep the bus idle for 74 clock cycles */
	return mmc_cmd(0, 0, NULL, INIT_CMD);
}

static int sd_init_card(struct mmc_cid *cid, int verbose)
{
	unsigned long resp[4];
	int i, ret = 0;

	mmc_idle_cards();
	for (i = 0; i < 1000; i++) {
		ret = mmc_acmd(MMC_ACMD_SD_SEND_OP_COND, CFG_MMC_OP_COND,
			       resp, R3 | NID);
		if (ret || (resp[0] & 0x80000000))
			break;
		ret = -ETIMEDOUT;
	}

	if (ret)
		return ret;

	ret = mmc_cmd(MMC_CMD_ALL_SEND_CID, 0, resp, R2 | NID);
	if (ret)
		return ret;
	sd_parse_cid(cid, resp);
	if (verbose)
		mmc_dump_cid(cid);

	/* Get RCA of the card that responded */
	ret = mmc_cmd(MMC_CMD_SD_SEND_RELATIVE_ADDR, 0, resp, R6 | NCR);
	if (ret)
		return ret;

	mmc_rca = resp[0] >> 16;
	if (verbose)
		printf("SD Card detected (RCA %u)\n", mmc_rca);
	mmc_card_is_sd = 1;
	return 0;
}

static int sdhc_init_card(struct mmc_cid *cid, int verbose)
{
    unsigned long resp[4];
    int i, ret = 0;

    mmc_idle_cards();

    ret = mmc_cmd(MMC_CMD_SD_SEND_IF_COND, 0x00000122, resp, R7 | NID);
    if (ret || resp[0]!=0x122)
	return ret;

    for (i = 0; i < 1000; i++) {
	ret = mmc_acmd(MMC_ACMD_SD_SEND_OP_COND, 0x40000000 ,
	       resp, R3 | NID);
	    if( ret==0 && (resp[0] & 0x80000000) && (resp[0] & 0x40000000)==0x40000000 )
            {
                mmc_card_is_sdhc = 1;
                break;
            }
            if (ret || (resp[0] & 0x80000000))
	        break;
            ret = -ETIMEDOUT;
    }

    if (ret)
	return ret;

    ret = mmc_cmd(MMC_CMD_ALL_SEND_CID, 0, resp, R2 | NID);
    if (ret)
	return ret;
    sd_parse_cid(cid, resp);
    if (verbose)
	mmc_dump_cid(cid);

    /* Get RCA of the card that responded */
    ret = mmc_cmd(MMC_CMD_SD_SEND_RELATIVE_ADDR, 0, resp, R6 | NCR);
    if (ret)
	return ret;

    mmc_rca = resp[0] >> 16;
    if (verbose)
	printf("SDv2/SDHC Card detected (RCA %u) SDHC flag %d\n", mmc_rca,mmc_card_is_sdhc);
    mmc_card_is_sd = 1;
    return 0;
}


static int mmc_init_card(struct mmc_cid *cid, int verbose)
{
	unsigned long resp[4];
	int i, ret = 0;

	mmc_idle_cards();
	for (i = 0; i < 1000; i++) {
		ret = mmc_cmd(MMC_CMD_SEND_OP_COND, CFG_MMC_OP_COND, resp,
			      R3 | NID | OPEN_DRAIN);
		if (ret || (resp[0] & 0x80000000))
			break;
		ret = -ETIMEDOUT;
	}

	if (ret)
		return ret;

	/* Get CID of all cards. FIXME: Support more than one card */
	ret = mmc_cmd(MMC_CMD_ALL_SEND_CID, 0, resp, R2 | NID | OPEN_DRAIN);
	if (ret)
		return ret;
	mmc_parse_cid(cid, resp);
	if (verbose)
		mmc_dump_cid(cid);

	/* Set Relative Address of the card that responded */
	ret = mmc_cmd(MMC_CMD_SET_RELATIVE_ADDR, mmc_rca << 16, resp,
		      R1 | NCR | OPEN_DRAIN);
	return ret;
}

static void mci_set_data_timeout(struct mmc_csd *csd)
{
	static const unsigned int dtomul_to_shift[] = {
		0, 4, 7, 8, 10, 12, 16, 20,
	};
	static const unsigned int taac_exp[] = {
		1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
	};
	static const unsigned int taac_mant[] = {
		0,  10, 12, 13, 15, 60, 25, 30,
		35, 40, 45, 50, 55, 60, 70, 80,
	};
	unsigned int timeout_ns, timeout_clks;
	unsigned int e, m;
	unsigned int dtocyc, dtomul;
	unsigned int shift;
	u32 dtor;

	e = csd->taac & 0x07;
	m = (csd->taac >> 3) & 0x0f;

	timeout_ns = (taac_exp[e] * taac_mant[m] + 9) / 10;
	timeout_clks = csd->nsac * 100;

	timeout_clks += (((timeout_ns + 9) / 10)
			 * ((CFG_MMC_CLK_PP + 99999) / 100000) + 9999) / 10000;
	if (!mmc_card_is_sd)
		timeout_clks *= 10;
	else
		timeout_clks *= 100;

	dtocyc = timeout_clks;
	dtomul = 0;
	while (dtocyc > 15 && dtomul < 8) {
		dtomul++;
		shift = dtomul_to_shift[dtomul];
		dtocyc = (timeout_clks + (1 << shift) - 1) >> shift;
	}

	if (dtomul >= 8) {
		dtomul = 7;
		dtocyc = 15;
		puts("Warning: Using maximum data timeout\n");
	}

	dtor = (MMCI_BF(DTOMUL, dtomul)
		| MMCI_BF(DTOCYC, dtocyc));
	mmci_writel(DTOR, dtor);

	printf("mmc: Using %u cycles data timeout (DTOR=0x%x)\n",
	       dtocyc << shift, dtor);
}

int mmc_init(int verbose)
{
	struct mmc_cid cid;
	struct mmc_csd csd;
	unsigned long csd_raw[4];
	unsigned int max_blksz;
	int ret;
	*AT91C_PIOA_PDR = (1<<27)|(1<<28)|(1<<29); /* GPIO to MMC Interface */
	*AT91C_PMC_PCER = 1 << 10;	    /* Peripheral Clock Enable Register */
	/* Initialize controller */
	mmci_writel(CR, MMCI_BIT(SWRST));
	mmci_writel(CR, MMCI_BIT(MCIEN));
	mmci_writel(DTOR, 0x5f);
	mmci_writel(IDR, ~0UL);
	mci_set_mode(CFG_MMC_CLK_OD, MMC_DEFAULT_BLKLEN);

	mmc_card_is_sd = 0;
        mmc_card_is_sdhc = 0;
        ret = sdhc_init_card(&cid,verbose);
        if(ret)
        {
	    ret = sd_init_card(&cid, verbose);
	    if (ret) 
            {
	        mmc_rca = MMC_DEFAULT_RCA;
	        ret = mmc_init_card(&cid, verbose);
            }
	}
	if (ret)
	    return ret;

	/* Get CSD from the card */
	ret = mmc_cmd(MMC_CMD_SEND_CSD, mmc_rca << 16, csd_raw, R2 | NCR);
	mmc_parse_csd(&csd,csd_raw);
	if (ret)
		return ret;
	if (verbose)
		mmc_dump_csd(&csd);

	mci_set_data_timeout(&csd);

	/* Initialize the blockdev structure */
	mmc_blkdev.if_type = IF_TYPE_MMC;
	mmc_blkdev.part_type = PART_TYPE_DOS;
	mmc_blkdev.block_read = mmc_bread;
	sprintf((char *)mmc_blkdev.vendor,
		"Man %02x%04x Snr %08x",
		cid.mid, cid.oid, cid.psn);
	strncpy((char *)mmc_blkdev.product, cid.pnm,
		sizeof(mmc_blkdev.product));
	sprintf((char *)mmc_blkdev.revision, "%x %x",
		cid.prv >> 4, cid.prv & 0x0f);

	/*
	 * If we can't use 512 byte blocks, refuse to deal with the
	 * card. Tons of code elsewhere seems to depend on this.
	 */
	max_blksz = 1 << csd.read_bl_len;
	if (max_blksz < 512 || (max_blksz > 512 && !csd.read_bl_partial)) {
		printf("Card does not support 512 byte reads, aborting.\n");
		return -ENODEV;
	}
	mmc_blkdev.blksz = 512;
	mmc_blkdev.lba = (csd.c_size + 1) * (1 << (csd.c_size_mult + 2));

	mci_set_mode(CFG_MMC_CLK_PP, mmc_blkdev.blksz);

#if 0
	if (fat_register_device(&mmc_blkdev, 1))
		printf("Could not register MMC fat device\n");
#else
	init_part(&mmc_blkdev);
#endif

	return 0;
}

int mmc_read(ulong src, uchar *dst, int size)
{
	return -ENOSYS;
}

int mmc_write(uchar *src, ulong dst, int size)
{
	return -ENOSYS;
}

int mmc2info(ulong addr)
{
	return 0;
}

#endif /* CONFIG_MMC */
