/* drivers/video/ssd1906fb.c
 *
 * (c) 2009 Kuba Odias <kuba.odias@gmail.com>
 *
 * Driver for Solomon Systech Limited SSD1906 framebuffer chip
 *
 * Adapted from
 *  linux/drivers/video/s1d13xxxfb.c
 *
 * TODO: 
 * 	- correct sleep times
 * 	- power saving mode
 *	- LCD Bias Power
 *	- clock speeds
 *	- backlight_enable() in ifdef
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file COPYING in the main directory of this archive for
 * more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/fb.h>

#include <asm/io.h>

#include <video/ssd1906fb.h>

#define PFX "ssd1906fb: "

#if 0
#define dbg(fmt, args...) do { printk(KERN_INFO fmt, ## args); } while(0)
#else
#define dbg(fmt, args...) do { } while (0)
#endif

/*
 * Here we define the default struct fb_fix_screeninfo
 */
static struct fb_fix_screeninfo __devinitdata ssd1906fb_fix = {
	.id		= SSD1906_FBID,
	.type		= FB_TYPE_PACKED_PIXELS,
	.visual		= FB_VISUAL_PSEUDOCOLOR,
	.xpanstep	= 0,
	.ypanstep	= 1,
	.ywrapstep	= 0,
	.accel		= FB_ACCEL_NONE,
};

static inline u8 ssd1906fb_readreg(struct ssd1906fb_par *par, u16 regno)
{
	return readb(par->regs + regno);
}

static inline void ssd1906fb_writereg(struct ssd1906fb_par *par, u16 regno, u8 value)
{
	writeb(value, par->regs + regno);
}

static inline void ssd1906fb_runinit(struct ssd1906fb_par *par,
			const struct ssd1906fb_regval *initregs,
			const unsigned int size)
{
	int i;

	for (i = 0; i < size; i++) 
	{
		ssd1906fb_writereg(par, initregs[i].addr, initregs[i].value);
        }

	/* make sure the hardware can cope with us */
	mdelay(1);
}

static inline void lcd_enable(struct ssd1906fb_par *par)
{
	/* disable power saving mode */
	u8 mode = ssd1906fb_readreg(par, SSD1906REG_PS_CONF);
	mode &= ~0x01;
	ssd1906fb_writereg(par, SSD1906REG_PS_CONF, mode);

	/* wait 20ns */
	ndelay(100);

	mode = ssd1906fb_readreg(par, SSD1906REG_DISP_MODE);
	mode &= ~0x80;
	ssd1906fb_writereg(par, SSD1906REG_DISP_MODE, mode);

	/* TODO: wait for min. 0.5ms and enable LCD bias power */
	udelay(1000);
}

static inline void lcd_disable(struct ssd1906fb_par *par)
{
	u8 mode;
	/* TODO: disable LCD bias power */

	/* wait for min. 10 frames (about 0.2s) and enable power saving mode */
	mdelay(400);

	mode = ssd1906fb_readreg(par, SSD1906REG_PS_CONF);
	mode |= 0x01;
	ssd1906fb_writereg(par, SSD1906REG_PS_CONF, mode);

	/* wait 20ns */
	ndelay(100);

	mode = ssd1906fb_readreg(par, SSD1906REG_DISP_MODE);
	mode |= 0x80;
	ssd1906fb_writereg(par, SSD1906REG_DISP_MODE, mode);
}

/* framebuffer control routines */

static inline void ssd1906fb_setup_pseudocolour(struct fb_info *info)
{
	info->fix.visual = FB_VISUAL_PSEUDOCOLOR;

	info->var.red.length = 4;
	info->var.green.length = 4;
	info->var.blue.length = 4;
}

static inline void ssd1906fb_setup_truecolour(struct fb_info *info)
{
	info->fix.visual = FB_VISUAL_TRUECOLOR;
	info->var.bits_per_pixel = 16;

	info->var.red.length = 5;
	info->var.red.offset = 11;

	info->var.green.length = 6;
	info->var.green.offset = 5;

	info->var.blue.length = 5;
	info->var.blue.offset = 0;
}

/**
 *      ssd1906fb_set_par - Alters the hardware state.
 *      @info: frame buffer structure
 *
 *	Using the fb_var_screeninfo in fb_info we set the depth of the
 *	framebuffer. This function alters the par AND the
 *	fb_fix_screeninfo stored in fb_info. It doesn't not alter var in
 *	fb_info since we are using that data. This means we depend on the
 *	data in var inside fb_info to be supported by the hardware.
 *	xxxfb_check_var is always called before xxxfb_set_par to ensure this.
 *
 *	XXX TODO: write proper ssd1906fb_check_var(), without which that
 *	function is quite useless.
 */
static int ssd1906fb_set_par(struct fb_info *info)
{
	struct ssd1906fb_par *ssd1906fb = info->par;
	unsigned int val;

	dbg("ssd1906fb_set_par: bpp=%d\n", info->var.bits_per_pixel);

	if (ssd1906fb->display & 0x01)
		val = ssd1906fb_readreg(ssd1906fb, SSD1906REG_DISP_MODE);   /* read colour control */
	else
		return -EINVAL;

	val &= ~0x07;

	switch (info->var.bits_per_pixel) {
		case 4:
			dbg("pseudo colour 4\n");
			ssd1906fb_setup_pseudocolour(info);
			val |= 2;
			break;
		case 8:
			dbg("pseudo colour 8\n");
			ssd1906fb_setup_pseudocolour(info);
			val |= 3;
			break;
		case 16:
			dbg("true colour\n");
			ssd1906fb_setup_truecolour(info);
			val |= 4;
			break;

		default:
			dbg("bpp not supported!\n");
			return -EINVAL;
	}

	dbg("writing %02x to display mode register\n", val);

	if (ssd1906fb->display & 0x01)
		ssd1906fb_writereg(ssd1906fb, SSD1906REG_DISP_MODE, val);

	info->fix.line_length  = info->var.xres * info->var.bits_per_pixel;
	info->fix.line_length /= 8;

	dbg("setting line_length to %d\n", info->fix.line_length);

	dbg("done setup\n");

	return 0;
}

/**
 *  	ssd1906fb_setcolreg - sets a color register.
 *      @regno: Which register in the CLUT we are programming
 *      @red: The red value which can be up to 16 bits wide
 *	@green: The green value which can be up to 16 bits wide
 *	@blue:  The blue value which can be up to 16 bits wide.
 *	@transp: If supported the alpha value which can be up to 16 bits wide.
 *      @info: frame buffer info structure
 *
 *	Returns negative errno on error, or zero on success.
 */
static int ssd1906fb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			u_int transp, struct fb_info *info)
{
	struct ssd1906fb_par *ssd1906fb = info->par;
	unsigned int pseudo_val;

	if (regno >= SSD1906_LUT_SIZE)
		return -EINVAL;

	dbg("ssd1906fb_setcolreg: %d: rgb=%d,%d,%d, tr=%d\n",
		    regno, red, green, blue, transp);

	if (info->var.grayscale)
		red = green = blue = (19595*red + 38470*green + 7471*blue) >> 16;

	switch (info->fix.visual) {
		case FB_VISUAL_TRUECOLOR:
			if (regno >= 16)
			{
				/* TODO: remove warning */
				printk(KERN_WARNING PFX "Warning: fix.visual=FB_VISUAL_TRUECOLOR, regno=%d\n", regno); 
				return -EINVAL;
			}

			/* deal with creating pseudo-palette entries */
			/* TODO: try 
				pseudo_val = (red >> (info->var.bits_per_pixel - info->var.red.length)) << info->var.red.offset;
			*/
			pseudo_val  = (red   >> 11) << info->var.red.offset;
			pseudo_val |= (green >> 10) << info->var.green.offset;
			pseudo_val |= (blue  >> 11) << info->var.blue.offset;

			dbg("ssd1906fb_setcolreg: pseudo %d, val %08x\n",
				    regno, pseudo_val);

			((u32 *)info->pseudo_palette)[regno] = pseudo_val;
			break;
		case FB_VISUAL_PSEUDOCOLOR:
			ssd1906fb_writereg(ssd1906fb, SSD1906REG_LUT_WRITE_ADDRESS, regno);
			ssd1906fb_writereg(ssd1906fb, SSD1906REG_LUT_RED_DATA_WRITE, red);
			ssd1906fb_writereg(ssd1906fb, SSD1906REG_LUT_GREEN_DATA_WRITE, green);
			ssd1906fb_writereg(ssd1906fb, SSD1906REG_LUT_BLUE_DATA_WRITE, blue);

			break;
		default:
			return -ENOSYS;
	}

	dbg("ssd1906fb_setcolreg: done\n");

	return 0;
}

/**
 *      ssd1906fb_blank - blanks the display.
 *      @blank_mode: the blank mode we want.
 *      @info: frame buffer structure that represents a single frame buffer
 *
 *      Blank the screen if blank_mode != 0, else unblank. Return 0 if
 *      blanking succeeded, != 0 if un-/blanking failed due to e.g. a
 *      video mode which doesn't support it. Implements VESA suspend
 *      and powerdown modes on hardware that supports disabling hsync/vsync:
 *      blank_mode == 2: suspend vsync
 *      blank_mode == 3: suspend hsync
 *      blank_mode == 4: powerdown
 *
 *      Returns negative errno on error, or zero on success.
 */
static int ssd1906fb_blank(int blank_mode, struct fb_info *info)
{
	struct ssd1906fb_par *par = info->par;

	dbg("ssd1906fb_blank: blank=%d, info=%p\n", blank_mode, info);

	switch (blank_mode) {
		case FB_BLANK_UNBLANK:
		case FB_BLANK_NORMAL:
			if ((par->display & 0x01) != 0)
				lcd_enable(par);
			break;
		case FB_BLANK_VSYNC_SUSPEND:
		case FB_BLANK_HSYNC_SUSPEND:
			break;
		case FB_BLANK_POWERDOWN:
			lcd_disable(par);
			break;
		default:
			return -EINVAL;
	}

	/* let fbcon do a soft blank for us */
	return ((blank_mode == FB_BLANK_NORMAL) ? 1 : 0);
}

/**
 *      ssd1906fb_pan_display - Pans the display.
 *      @var: frame buffer variable screen structure
 *      @info: frame buffer structure that represents a single frame buffer
 *
 *	Pan (or wrap, depending on the `vmode' field) the display using the
 *  	`yoffset' field of the `var' structure (`xoffset'  not yet supported).
 *  	If the values don't fit, return -EINVAL.
 *
 *      Returns negative errno on error, or zero on success.
 */
static int ssd1906fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct ssd1906fb_par *par = info->par;
	u32 start;

	if (var->xoffset != 0)	/* not yet ... */
	{
		return -EINVAL;
	}

	if (var->yoffset + info->var.yres > info->var.yres_virtual)
	{
		return -EINVAL;
	}

	start = (info->fix.line_length >> 1) * var->yoffset;

	if ((par->display & 0x01)) 
	{
		ssd1906fb_writereg(par, SSD1906REG_DISP_START0, (start & 0xff));
		ssd1906fb_writereg(par, SSD1906REG_DISP_START1, ((start >> 8) & 0xff));
		ssd1906fb_writereg(par, SSD1906REG_DISP_START2, ((start >> 16) & 0x0f));
	}
	else
	{
		printk(KERN_WARNING PFX "Warning: Display is not LCD (par->display=0x%x)\n", par->display); 
		return -EINVAL;
	}

	return 0;
}


/* framebuffer information structures */

static struct fb_ops ssd1906fb_fbops = {
	.owner		= THIS_MODULE,
	.fb_set_par	= ssd1906fb_set_par,
	.fb_setcolreg	= ssd1906fb_setcolreg,
	.fb_blank	= ssd1906fb_blank,

	.fb_pan_display	= ssd1906fb_pan_display,

	/* to be replaced by any acceleration we can */
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

/**
 *      ssd1906fb_fetch_hw_state - Configure the framebuffer according to
 *	hardware setup.
 *      @info: frame buffer structure
 *
 *	We setup the framebuffer structures according to the current
 *	hardware setup. On some machines, the BIOS will have filled
 *	the chip registers with such info, on others, these values will
 *	have been written in some init procedure. In any case, the
 *	software values needs to match the hardware ones. This is what
 *	this function ensures.
 *
 *	Note: some of the hardcoded values here might need some love to
 *	work on various chips, and might need to no longer be hardcoded.
 */
static void __devinit ssd1906fb_fetch_hw_state(struct fb_info *info)
{
	struct fb_var_screeninfo *var = &info->var;
	struct fb_fix_screeninfo *fix = &info->fix;
	struct ssd1906fb_par *par = info->par;
	u8 panel, display;
	u16 offset;
	u32 xres, yres;
	int bpp;
	int is_color;

	fix->type = FB_TYPE_PACKED_PIXELS;

	/* general info */
	par->display = ssd1906fb_readreg(par, SSD1906REG_DISP_MODE);

	display = ssd1906fb_readreg(par, SSD1906REG_DISP_MODE);

	bpp = display & 0x07;

	switch (bpp) {
		case 2:	/* 4 bpp */
		case 3:	/* 8 bpp */
			var->bits_per_pixel = 8;
			var->red.offset = var->green.offset = var->blue.offset = 0;
			var->red.length = var->green.length = var->blue.length = 8;
			break;
		case 4:	/* 16 bpp */
			ssd1906fb_setup_truecolour(info);
			break;
		default:
			dbg("bpp: %i\n", bpp);
	}
	fb_alloc_cmap(&info->cmap, 256, 0);

	/* LCD info */
	panel = ssd1906fb_readreg(par, SSD1906REG_PANEL_TYPE);
	is_color = (panel & 0x40) != 0;

	xres = (ssd1906fb_readreg(par, SSD1906REG_HDISP_PERIOD) + 1) * 8;
	yres = (ssd1906fb_readreg(par, SSD1906REG_VDISP_PERIOD0) +
		((ssd1906fb_readreg(par, SSD1906REG_VDISP_PERIOD1) & 0x03) << 8) + 1);

	/* TODO: what is this offset for? */
	/*offset = (ssd1906fb_readreg(par, S1DREG_LCD_MEM_OFF0) +
		((ssd1906fb_readreg(par, S1DREG_LCD_MEM_OFF1) & 0x7) << 8));

	xres_virtual = offset * 16 / var->bits_per_pixel;
	yres_virtual = fix->smem_len / (offset * 2);*/

	var->xres		= xres;
	var->yres		= yres;
	/*var->xres_virtual	= xres_virtual;
	var->yres_virtual	= yres_virtual;*/
	var->xoffset		= var->yoffset = 0;

	fix->line_length	= offset * 2;

	var->grayscale		= !is_color;

	var->activate		= FB_ACTIVATE_NOW;

	dbg(PFX "bpp=%d, xres=%d, yres=%d, is_color=%d\n",
		var->bits_per_pixel, xres, yres, is_color);
}


static int ssd1906fb_remove(struct platform_device *pdev)
{
	struct fb_info *info = platform_get_drvdata(pdev);
	struct ssd1906fb_par *par = NULL;

	if (info) {
		par = info->par;
		if (par && par->regs) {
			/* disable output & enable powersave */
			lcd_disable(par);
			iounmap(par->regs);
		}

		fb_dealloc_cmap(&info->cmap);

		if (info->screen_base)
			iounmap(info->screen_base);

		framebuffer_release(info);
	}

	release_mem_region(pdev->resource[0].start,
			pdev->resource[0].end - pdev->resource[0].start +1);
	release_mem_region(pdev->resource[1].start,
			pdev->resource[1].end - pdev->resource[1].start +1);
	return 0;
}

static int __devinit ssd1906fb_probe(struct platform_device *pdev)
{
	struct ssd1906fb_par *default_par;
	struct fb_info *info;
	struct ssd1906fb_pdata *pdata = NULL;
	u8 revision;

	dbg("probe called: device is %p\n", pdev);

	printk(KERN_INFO "Solomon Systech Limited SSD1906 FB Driver\n");

	/* enable platform-dependent hardware glue, if any */
	if (pdev->dev.platform_data)
		pdata = pdev->dev.platform_data;

	if (pdata && pdata->platform_init_video)
		pdata->platform_init_video();


	if (pdev->num_resources != 2)
	{
		dev_err(&pdev->dev, "invalid num_resources: %i\n",
		       pdev->num_resources);
		ssd1906fb_remove(pdev);
		return -ENODEV;
	}

	/* resource[0] is RAM, resource[1] is registers */
	if ((pdev->resource[0].flags != IORESOURCE_MEM) || (pdev->resource[1].flags != IORESOURCE_MEM))
	{
		dev_err(&pdev->dev, "invalid resource type\n");
		ssd1906fb_remove(pdev);
		return -ENODEV;
	}

	if (!request_mem_region(pdev->resource[0].start,
		pdev->resource[0].end - pdev->resource[0].start +1, "ssd1906fb mem"))
	{
		dev_dbg(&pdev->dev, "request_mem_region failed\n");
		ssd1906fb_remove(pdev);
		return -EBUSY;
	}

	if (!request_mem_region(pdev->resource[1].start,
		pdev->resource[1].end - pdev->resource[1].start +1, "ssd1906fb regs"))
	{
		dev_dbg(&pdev->dev, "request_mem_region failed\n");
		ssd1906fb_remove(pdev);
		return -EBUSY;
	}

	info = framebuffer_alloc(sizeof(struct ssd1906fb_par) + sizeof(u32) * 256, &pdev->dev);
	if (!info)
	{
		ssd1906fb_remove(pdev);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, info);
	default_par = info->par;
	default_par->regs = ioremap_nocache(pdev->resource[1].start,
			pdev->resource[1].end - pdev->resource[1].start +1);
	if (!default_par->regs)
	{
		printk(KERN_ERR PFX "unable to map registers\n");
		ssd1906fb_remove(pdev);
		return -ENOMEM;
	}
	info->pseudo_palette = default_par->pseudo_palette;

	info->screen_base = ioremap_nocache(pdev->resource[0].start,
			pdev->resource[0].end - pdev->resource[0].start +1);

	if (!info->screen_base)
	{
		printk(KERN_ERR PFX "unable to map framebuffer\n");
		ssd1906fb_remove(pdev);
		return -ENOMEM;
	}

	revision = ssd1906fb_readreg(default_par, SSD1906REG_REV_CODE);
	if ((revision >> 2) != SSD1906_CHIP_REV)
	{
		printk(KERN_INFO PFX "chip not found: %i\n", (revision >> 2));
		ssd1906fb_remove(pdev);
		return -ENODEV;
	}

	info->fix = ssd1906fb_fix;
	info->fix.mmio_start = pdev->resource[1].start;
	info->fix.mmio_len = pdev->resource[1].end - pdev->resource[1].start + 1;
	info->fix.smem_start = pdev->resource[0].start;
	info->fix.smem_len = pdev->resource[0].end - pdev->resource[0].start + 1;

	printk(KERN_INFO PFX "regs mapped at 0x%p, fb %d KiB mapped at 0x%p\n",
	       default_par->regs, info->fix.smem_len / 1024, info->screen_base);

	info->par = default_par;
	info->fbops = &ssd1906fb_fbops;
	info->flags = FBINFO_DEFAULT | FBINFO_HWACCEL_YPAN;

	/* perform "manual" chip initialization, if needed */
	if (pdata && pdata->initregs)
		ssd1906fb_runinit(info->par, pdata->initregs, pdata->initregssize);

	ssd1906fb_fetch_hw_state(info);

	if (register_framebuffer(info) < 0)
	{
		ssd1906fb_remove(pdev);
		return -EINVAL;
	}

	printk(KERN_INFO "fb%d: %s frame buffer device\n",
	       info->node, info->fix.id);

	return 0;
}

static struct platform_driver ssd1906fb_driver = {
	.probe		= ssd1906fb_probe,
	.remove		= ssd1906fb_remove,
	.driver		= {
		.name	= SSD1906_DEVICENAME,
	},
};


static int __init ssd1906fb_init(void)
{

#ifndef MODULE
	if (fb_get_options("ssd1906fb", NULL))
		return -ENODEV;
#endif

	return platform_driver_register(&ssd1906fb_driver);
}


static void __exit ssd1906fb_exit(void)
{
	platform_driver_unregister(&ssd1906fb_driver);
}

module_init(ssd1906fb_init);
module_exit(ssd1906fb_exit);


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Framebuffer driver for SSD1906");
MODULE_AUTHOR("Kuba Odias <kuba.odias@gmail.com>");
