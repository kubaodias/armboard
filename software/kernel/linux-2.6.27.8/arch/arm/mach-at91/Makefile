#
# Makefile for the linux kernel.
#

obj-y		:= irq.o gpio.o
obj-m		:=
obj-n		:=
obj-		:=

obj-$(CONFIG_AT91_PMC_UNIT)	+= clock.o

# CPU-specific support
obj-$(CONFIG_ARCH_AT91RM9200)	+= at91rm9200.o at91rm9200_time.o at91rm9200_devices.o
obj-$(CONFIG_ARCH_AT91SAM9260)	+= at91sam9260.o at91sam926x_time.o at91sam9260_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9261)	+= at91sam9261.o at91sam926x_time.o at91sam9261_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9263)	+= at91sam9263.o at91sam926x_time.o at91sam9263_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9RL)	+= at91sam9rl.o at91sam926x_time.o at91sam9rl_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9G20)	+= at91sam9260.o at91sam926x_time.o at91sam9260_devices.o  sam9_smc.o
obj-$(CONFIG_ARCH_AT91CAP9)	+= at91cap9.o at91sam926x_time.o at91cap9_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91X40)	+= at91x40.o at91x40_time.o

# AT91RM9200 board-specific support
obj-$(CONFIG_MACH_ONEARM)	+= board-1arm.o
obj-$(CONFIG_ARCH_AT91RM9200DK)	+= board-dk.o
obj-$(CONFIG_MACH_AT91RM9200EK)	+= board-ek.o
obj-$(CONFIG_MACH_CSB337)	+= board-csb337.o
obj-$(CONFIG_MACH_CSB637)	+= board-csb637.o
obj-$(CONFIG_MACH_CARMEVA)	+= board-carmeva.o
obj-$(CONFIG_MACH_KB9200)	+= board-kb9202.o
obj-$(CONFIG_MACH_ATEB9200)	+= board-eb9200.o
obj-$(CONFIG_MACH_KAFA)		+= board-kafa.o
obj-$(CONFIG_MACH_PICOTUX2XX)	+= board-picotux200.o
obj-$(CONFIG_MACH_ECBAT91)	+= board-ecbat91.o
obj-$(CONFIG_MACH_CHUB)		+= board-chub.o
obj-$(CONFIG_MACH_HOMEMATIC)	+= board-homematic.o
obj-$(CONFIG_MACH_SWEDATMS)	+= board-tms.o
obj-$(CONFIG_MACH_TT9200)	+= board-tt9200.o
obj-$(CONFIG_MACH_YL9200)	+= board-yl-9200.o
obj-$(CONFIG_MACH_LAB_AT91RM9200) += board-lab-at91rm9200.o

# AT91SAM9260 board-specific support
obj-$(CONFIG_MACH_AT91SAM9260EK) += board-sam9260ek.o
obj-$(CONFIG_MACH_CAM60)	+= board-cam60.o
obj-$(CONFIG_MACH_SAM9_L9260)	+= board-sam9-l9260.o
obj-$(CONFIG_MACH_USB_A9260)	+= board-usb-a9260.o
obj-$(CONFIG_MACH_QIL_A9260)	+= board-qil-a9260.o

# AT91SAM9261 board-specific support
obj-$(CONFIG_MACH_AT91SAM9261EK) += board-sam9261ek.o

# AT91SAM9263 board-specific support
obj-$(CONFIG_MACH_AT91SAM9263EK) += board-sam9263ek.o
obj-$(CONFIG_MACH_USB_A9263)	+= board-usb-a9263.o
obj-$(CONFIG_MACH_CSB737)	+= board-csb737.o
obj-$(CONFIG_MACH_TOTEMNOVA)	+= board-totemnova.o
obj-$(CONFIG_MACH_NEOCORE926)	+= board-neocore926.o

# AT91SAM9RL board-specific support
obj-$(CONFIG_MACH_AT91SAM9RLEK)	+= board-sam9rlek.o

# AT91SAM9G20 board-specific support
obj-$(CONFIG_MACH_AT91SAM9G20EK) += board-sam9g20ek.o

# AT91CAP9 board-specific support
obj-$(CONFIG_MACH_AT91CAP9ADK)	+= board-cap9adk.o

# AT91X40 board-specific support
obj-$(CONFIG_MACH_AT91EB01)	+= board-eb01.o

# Drivers
obj-y				+= leds.o
obj-$(CONFIG_FB_S1D13XXX)	+= ics1523.o

# Power Management
obj-$(CONFIG_PM)		+= pm.o
obj-$(CONFIG_AT91_SLOW_CLOCK)	+= pm_slowclock.o

ifeq ($(CONFIG_PM_DEBUG),y)
CFLAGS_pm.o += -DDEBUG
endif
