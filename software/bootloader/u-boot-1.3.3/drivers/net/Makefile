#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB 	:= $(obj)libnet.a

COBJS-y += 3c589.o
COBJS-y += bcm570x.o bcm570x_autoneg.o 5701rls.o
COBJS-$(CONFIG_BFIN_MAC) += bfin_mac.o
COBJS-y += cs8900.o
COBJS-y += dc2114x.o
COBJS-y += dm9000x.o
COBJS-y += enc28j60.o
COBJS-y += fsl_mcdmafec.o
COBJS-$(CONFIG_GRETH) += greth.o
COBJS-y += inca-ip_sw.o
COBJS-y += ks8695eth.o
COBJS-y += lan91c96.o
COBJS-y += macb.o
COBJS-y += mcffec.o
ifeq ($(CONFIG_DRIVER_NE2000),y)
COBJS-y += ne2000.o
COBJS-$(CONFIG_DRIVER_AX88796L) += ax88796.o
endif
COBJS-y += netarm_eth.o
COBJS-y += netconsole.o
COBJS-y += plb2800_eth.o
COBJS-y += s3c4510b_eth.o
COBJS-y += smc91111.o
COBJS-y += smc911x.o
COBJS-y += tigon3.o
COBJS-y += tsec.o
COBJS-y += tsi108_eth.o
COBJS-y += vsc7385.o
COBJS-$(CONFIG_XILINX_EMAC) += xilinx_emac.o
COBJS-$(CONFIG_XILINX_EMACLITE) += xilinx_emaclite.o

COBJS	:= $(COBJS-y)
SRCS 	:= $(COBJS:.o=.c)
OBJS 	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
