/*
 * (C) Copyright 2007
 * Author : Grzegorz Rajtar (McGregor) (mcgregor@blackmesaeast.com.pl)
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
//#define DEBUG_ETHER

#include <at91rm9200_net.h>
#include <net.h>
#include <ste100p.h>

#ifdef CONFIG_DRIVER_ETHER

#if defined(CONFIG_CMD_NET)

static void PhyReset(AT91PS_EMAC p_mac)
{
    unsigned short IntValue;
    unsigned int timeout;
#ifdef DEBUG_ETHER
	printf("ste100p PhyReset \n");
#endif
	at91rm9200_EmacEnableMDIO (p_mac);


	// first software reset the STE100P      
	at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &IntValue);
	udelay(1000);
	IntValue |= STE100P_XCR_RESET;
	
	at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &IntValue);
	udelay(10000);	
	timeout = 100;
	while (--timeout)
	{
        	at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &IntValue);
		if ((IntValue & STE100P_XCR_RESET) != STE100P_XCR_RESET)
		{
#ifdef DEBUG_ETHER		    
		    printf("PHY Reset OK\n");
#endif
		    break;
		}
		udelay(1000);
	}

	
	IntValue =  STE100P_ANA_FC | STE100P_ANA_TXF | STE100P_ANA_TXH |
			STE100P_ANA_10F | STE100P_ANA_10H;

	
	at91rm9200_EmacWritePhy (p_mac, STE100P_ANA_REG, &IntValue);	

	//default configuration
#ifdef  CONFIG_STE100P_OVERRIDE_HARDWARE
	IntValue = STE100P_100CTR_ENDCR | STE100P_100CTR_ENRZI | STE100P_100CTR_EN4B5B;
	IntValue &= ~(STE100P_100CTR_DISRER);
	IntValue &= ~(STE100P_100CTR_ISOTX);
	IntValue &= ~(STE100P_100CTR_DISMLT);
	IntValue &= ~(STE100P_100CTR_DISCRM);

	at91rm9200_EmacWritePhy (p_mac, STE100P_100CTR_REG, &IntValue);	

#endif //CONFIG_STE100P_OVERRIDE_HARDWARE

	/* Disable PHY Interrupts */
	
	at91rm9200_EmacReadPhy (p_mac, STE100P_XIE_REG, &IntValue);
	udelay(10000);
	/* disable all interrypts from STE100P */

	IntValue &= ~(STE100P_XIE_ANCE | STE100P_XIE_RFE | STE100P_XIE_LDE |
		STE100P_XIE_ANAE | STE100P_XIE_PDFE | STE100P_XIE_ANPE | STE100P_XIE_REFE);

	at91rm9200_EmacWritePhy (p_mac, STE100P_XIE_REG, &IntValue);
	udelay(10000);	

	at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &IntValue);
	
	IntValue |= STE100P_XCR_AN | STE100P_XCR_RSTRT_AN;
	at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &IntValue);

	at91rm9200_EmacDisableMDIO (p_mac);
}

/*
 * Name:
 *	ste100p_Isolate
 * Description:
 *	Isolates PHY 
 * Arguments:
 *	p_mac - pointer to AT91S_EMAC struct
 * Return value:
 *	TRUE - if id isolated successfuly
 *	FALSE- if error
 */

unsigned int ste100p_Isolate (AT91PS_EMAC p_mac)
{
    unsigned int result = FALSE;
    unsigned short IntValue;

    at91rm9200_EmacEnableMDIO (p_mac);
    udelay(10000);
    at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &IntValue);

    IntValue |= STE100P_XCR_ISOLATE ;//| STE100P_XCR_PWRDN;
    //IntValue &= ~STE100P_XCR_RESET;
    //IntValue &= ~STE100P_XCR_AN;

    result = at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &IntValue);
    udelay(10000);
    //Isolate is latch so we need to read once more the register
    at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &IntValue);
    at91rm9200_EmacDisableMDIO (p_mac);

#ifdef DEBUG_ETHER
    printf("ste100p_Isolate [%d]\n", result);
#endif
    return result;
}


/*
 * Name:
 *	ste100p_IsPhyConnected
 * Description:
 *	Reads the 2 PHY ID registers
 * Arguments:
 *	p_mac - pointer to AT91S_EMAC struct
 * Return value:
 *	TRUE - if id read successfully
 *	FALSE- if error
 */
unsigned int ste100p_IsPhyConnected (AT91PS_EMAC p_mac)
{
	unsigned short Id1, Id2;
	unsigned int result = FALSE;
	unsigned int tries = 100;

	at91rm9200_EmacEnableMDIO (p_mac);
	udelay(10000);
	do
	{	
	    Id1 = Id2 = 0;
	    udelay(10000);
	    at91rm9200_EmacReadPhy (p_mac, STE100P_PID1_REG, &Id1);

	    udelay(10000);
	    at91rm9200_EmacReadPhy (p_mac, STE100P_PID2_REG, &Id2);
	
	    Id2 = (Id2 & STE100P_PID2_PHYID_MASK) >> 6;

	    if ((Id1 == STE100P_PID1_PHYID_VAL) && (Id2 == STE100P_PID2_PHYID_VAL)) 
		result = TRUE;

	    tries--;
	} while ((!result) && (tries > 0));
	
	if (tries == 0)
		return FALSE;

	at91rm9200_EmacDisableMDIO (p_mac);		
#ifdef DEBUG_ETHER	
	printf ("ste100p id1[0x%02x] id2[0x%02x]\r\n", Id1, Id2);
#endif	
	return result;
}

/*
 * Name:
 *	ste100p_GetLinkSpeed
 * Description:
 *	Link parallel detection status of MAC is checked and set in the
 *	MAC configuration registers
 * Arguments:
 *	p_mac - pointer to MAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
UCHAR ste100p_GetLinkSpeed (AT91PS_EMAC p_mac)
{
	unsigned short stat;
	int result = 0;

	result = at91rm9200_EmacReadPhy (p_mac, STE100P_XSR_REG, &stat);

	if (!result)
		return FALSE;

	if (!(stat & STE100P_XSR_LINK))	/* link status up? */
	{ //last link failure is latched so reread STE100P_XSR_REG for new value
		result = at91rm9200_EmacReadPhy (p_mac, STE100P_XSR_REG, &stat);
		if (!result || !(stat & STE100P_XSR_LINK))
			return FALSE;
	}

        if (stat & STE100P_XSR_100TX_FULL) {
                /*set Emac for 100BaseTX and Full Duplex  */
                p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
                return TRUE;
        }

        if (stat & STE100P_XSR_100TX) {
                /*set Emac for 100BaseTX and Half Duplex  */
                p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
                                ~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
                                | AT91C_EMAC_SPD;
                return TRUE;
        }

        if (stat & STE100P_XSR_10T_FULL) {
                /*set MII for 10BaseT and Full Duplex  */
                p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
                                ~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
                                | AT91C_EMAC_FD;
                return TRUE;
        }

        if (stat & STE100P_XSR_10T) {
                /*set MII for 10BaseT and Half Duplex  */
                p_mac->EMAC_CFG &= ~(AT91C_EMAC_SPD | AT91C_EMAC_FD);
                return TRUE;
        }

	return FALSE;
}


/*
 * Name:
 *	ste100p_Deisolate
 * Description:
 *	deisolates PHY
 * Arguments:
 *	p_mac - pointer to struct AT91S_EMAC
 */

void ste100p_Deisolate (AT91PS_EMAC p_mac)
{
	unsigned short IntValue;

	IntValue = STE100P_XCR_SPEED | STE100P_XCR_AN |
		 STE100P_XCR_COLLEN;

	at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &IntValue);

	udelay(10000);
	IntValue = STE100P_100CTR_EN4B5B | STE100P_100CTR_ENRZI | 
			STE100P_100CTR_ENDCR;
	
	at91rm9200_EmacWritePhy (p_mac, STE100P_100CTR_REG, &IntValue);
	udelay(10000);


	if ((IntValue & STE100P_XCR_ISOLATE) == STE100P_XCR_ISOLATE)
	{	
	    IntValue &= ~STE100P_XCR_ISOLATE;
	    at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &IntValue);
	    udelay(10000);	
	    //isolate is latch so read once more the register
	    at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &IntValue);	
	    udelay(10000);
	}
}

/*
 * Name:
 *	ste100p_WaitForLink
 * Description:
 *	waits for link with timeout
 * Arguments:
 *	p_mac - pointer to struct AT91S_EMAC
 *	timeout - timeout in miliseconds
 */

UCHAR ste100p_WaitForLink (AT91PS_EMAC p_mac, unsigned long timeout)
{
	unsigned long loop ;
	unsigned short IntValue;

	loop = 0;
	do
	{
	    at91rm9200_EmacReadPhy (p_mac, STE100P_XSR_REG, &IntValue);
	    if (IntValue & STE100P_XSR_LINK)
		return TRUE;
	
	    udelay(1000);
	    loop++;
	    if (loop > timeout)
		break;
		
	} while (1);

	return FALSE;
}

/*
 * Name:
 *	ste100p_InitPhy
 * Description:
 *	MAC starts checking its link by using parallel detection and
 *	Autonegotiation and the same is set in the MAC configuration registers
 * Arguments:
 *	p_mac - pointer to struct AT91S_EMAC
 * Return value:
 *	TRUE - if link status set succesfully
 *	FALSE - if link status not set
 */
UCHAR ste100p_InitPhy (AT91PS_EMAC p_mac)
{
	UCHAR ret = FALSE;
	unsigned short IntValue;
	int aneg_status;

	PhyReset(p_mac);	
	
	at91rm9200_EmacEnableMDIO (p_mac);	

	ste100p_Deisolate(p_mac);

	at91rm9200_EmacDisableMDIO (p_mac);	
		

	
#if 1
	at91rm9200_EmacEnableMDIO (p_mac);
	
	ste100p_WaitForLink(p_mac, 500 /* timeout in ms */);	
		
	ret = ste100p_GetLinkSpeed (p_mac);

	if (!ret)
	{
	    ste100p_AutoNegotiate(p_mac, &aneg_status);
#ifdef DEBUG_ETHER
	    if (aneg_status)
	    {
		    printf("link speed autonegotiated\n");
		    ret = ste100p_GetLinkSpeed (p_mac);
	    }
	    else
		    printf("auto-neogtiation failed\n");
#endif //DEBUG_ETHER		    
	}

	/* Disable PHY Interrupts */
	
	at91rm9200_EmacReadPhy (p_mac, STE100P_XIE_REG, &IntValue);
	udelay(1000);
	/* disable all interrypts from SE100P */

	IntValue &= ~(STE100P_XIE_ANCE | STE100P_XIE_RFE | STE100P_XIE_LDE |
		STE100P_XIE_ANAE | STE100P_XIE_PDFE | STE100P_XIE_ANPE | STE100P_XIE_REFE);

	at91rm9200_EmacWritePhy (p_mac, STE100P_XIE_REG, &IntValue);
	udelay(10000);	
	

	
	at91rm9200_EmacDisableMDIO (p_mac);
	udelay(1000);	
#endif

#ifdef DEBUG_ETHER
	printf("ste100p InitPhy [");
	if (ret)
	    printf("OK]\n");
	else
	    printf("FAILED]\n");
#endif //DEBUG_ETHER
	return (ret);
}


/*
 * Name:
 *	ste100p_AutoNegotiate
 * Description:
 *	MAC Autonegotiates with the partner status of same is set in the
 *	MAC configuration registers
 * Arguments:
 *	dev - pointer to struct net_device
 * Return value:
 *	TRUE - if link status set successfully
 *	FALSE - if link status not set
 */
UCHAR ste100p_AutoNegotiate (AT91PS_EMAC p_mac, int *status)
{
	unsigned short value;
	unsigned short PhyAnar;
	unsigned short PhyAnalpar;
#ifdef DEBUG_ETHER	
	printf("ste100p AutoNegotiate\n");	
#endif //DEBUG_ETHER
#if 1
	/* Set ste100p control register */
	if (!at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &value))
		return FALSE;
	
	value &= ~STE100P_XCR_AN;	/* remove autonegotiation enable */
	value |= STE100P_XCR_ISOLATE;	/* Electrically isolate PHY */
	if (!at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &value))
		return FALSE;

	if (!at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &value))
		return FALSE;


	/* Set the Auto_negotiation Advertisement Register */
	/* MII advertising for Next page, 100BaseTxFD and HD, 10BaseTFD and HD, IEEE 802.3 */
	PhyAnar = STE100P_ANA_NXTPG | STE100P_ANA_TXF | STE100P_ANA_TXH |
		  STE100P_ANA_10F | STE100P_ANA_10H | STE100P_ANA_SF;
	if (!at91rm9200_EmacWritePhy (p_mac, STE100P_ANA_REG, &PhyAnar))
		return FALSE;

	/* Read the Control Register     */
	if (!at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &value))
		return FALSE;

	value |= STE100P_XCR_SPEED | STE100P_XCR_AN | STE100P_XCR_FULL_DUP;
	if (!at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &value))
		return FALSE;
	
	/* Restart Auto_negotiation  */
	value |= STE100P_XCR_AN;
	value &= ~STE100P_XCR_ISOLATE;
	value |= STE100P_XCR_RSTRT_AN;
	
	if (!at91rm9200_EmacWritePhy (p_mac, STE100P_XCR_REG, &value))
		return FALSE;
	udelay(10000);
	if (!at91rm9200_EmacReadPhy (p_mac, STE100P_XCR_REG, &value))
		return FALSE;

	/*check AutoNegotiate complete */
	udelay (10000);
	at91rm9200_EmacReadPhy (p_mac, STE100P_XSR_REG, &value);
	if (!(value & STE100P_XSR_AN_COMPLETE))
		return FALSE;

	/* Get the AutoNeg Link partner base page */
	if (!at91rm9200_EmacReadPhy (p_mac, STE100P_ANLP_REG, &PhyAnalpar))
		return FALSE;

	if ((PhyAnar & STE100P_ANA_TXF) && (PhyAnalpar & STE100P_ANLP_LPTXF)) {
		/*set MII for 100BaseTX and Full Duplex  */
		p_mac->EMAC_CFG |= AT91C_EMAC_SPD | AT91C_EMAC_FD;
		return TRUE;
	}

	if ((PhyAnar & STE100P_ANA_10F) && (PhyAnalpar & STE100P_ANLP_LPTXH)) {
		/*set MII for 10BaseT and Full Duplex  */
		p_mac->EMAC_CFG = (p_mac->EMAC_CFG &
				~(AT91C_EMAC_SPD | AT91C_EMAC_FD))
				| AT91C_EMAC_FD;
		return TRUE;
	}
#endif	
	return FALSE;
}

/*
 * Name:
 *	ste100p_DisableInterrupts
 * Description:
 *	disables interrupts
 * Arguments:
 *	p_mac - pointer to AT91S_EMAC struct
 */
void ste100p_DisableInterrupts (AT91PS_EMAC p_mac)
{

    unsigned short IntValue;
    unsigned int rep;

    rep = 0;

    at91rm9200_EmacEnableMDIO (p_mac);

    /* Disable PHY Interrupts */
	
    at91rm9200_EmacReadPhy (p_mac, STE100P_XIE_REG, &IntValue);
    udelay(10000);
    /* disable all interrypts from SE100P */

    IntValue &= ~(STE100P_XIE_ANCE | STE100P_XIE_RFE | STE100P_XIE_LDE |
    	STE100P_XIE_ANAE | STE100P_XIE_PDFE | STE100P_XIE_ANPE | STE100P_XIE_REFE);

    at91rm9200_EmacWritePhy (p_mac, STE100P_XIE_REG, &IntValue);
    udelay(10000);	

    IntValue = 1;
	
    do
    {
        at91rm9200_EmacReadPhy (p_mac, STE100P_XCSIIS_REG, &IntValue);
	rep++;
    }  while (IntValue != 0 && rep < 100);
	
    at91rm9200_EmacDisableMDIO (p_mac);
}


#endif	/* CONFIG_COMMANDS & CFG_CMD_NET */

#endif	/* CONFIG_DRIVER_ETHER */

