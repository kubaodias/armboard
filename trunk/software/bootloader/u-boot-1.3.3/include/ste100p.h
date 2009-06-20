#ifndef _STE100P_H_
#define _STE100P_H_


// UTILS
#define Bit(n) (1<<(n))


// STE100P register offsets.

#define	STE100P_XCR_REG 	0x00
#define	STE100P_XSR_REG 	0x01
#define	STE100P_PID1_REG    	0x02
#define	STE100P_PID2_REG    	0x03
#define	STE100P_ANA_REG    	0x04
#define	STE100P_ANLP_REG 	0x05
#define	STE100P_ANE_REG    	0x06
#define	STE100P_XCSIIS_REG    	0x11
#define	STE100P_XIE_REG    	0x12
#define	STE100P_100CTR_REG    	0x13
#define	STE100P_XMC_REG    	0x14


// STE100P XCR - Control register bit defines.
#define STE100P_XCR_RESET        0x8000                                                //(RW)
#define STE100P_XCR_LOOPBACK     0x4000                                                //(RW)
#define STE100P_XCR_SPEED        0x2000  // 1=100Meg, 0=10Meg                          (RW)
#define STE100P_XCR_AN           0x1000  // 1=Enable auto negotiation, 0=disable it    (RW)
#define STE100P_XCR_PWRDN        0x0800  // 1=Enable power down                        (RW)
#define STE100P_XCR_ISOLATE      0x0400  // 1=Isolate PHY from MII                     (RW)
#define STE100P_XCR_RSTRT_AN     0x0200  // 1=Restart Auto Negotioation process        (RW)
#define STE100P_XCR_FULL_DUP     0x0100  // 1=Enable full duplex mode, 0=half dup      (RW)
#define STE100P_XCR_COLLEN       0x0080  // 1=Collision test control                   (RW)

// STE100P XSR - Control status bit defines.

#define STE100P_XSR_100T4        Bit(15)                                           //           (RO)
#define STE100P_XSR_100TX_FULL   Bit(14)                                           //           (RO)
#define STE100P_XSR_100TX        Bit(13)                                           //           (RO)
#define STE100P_XSR_10T_FULL     Bit(12)                                           //           (RO)
#define STE100P_XSR_10T          Bit(11)                                           //           (RO)
#define STE100P_XSR_MFPS         Bit(6)    //MF preamble suppression                            (RO)
#define STE100P_XSR_AN_COMPLETE  Bit(5)
#define STE100P_XSR_RF           Bit(4)    //RF result of remote fault detection                (RO/LH)
#define STE100P_XSR_AN           Bit(3)    //AN - auto-negotation ability, always 1 for STE100P (RO)
#define STE100P_XSR_LINK         Bit(2)    // Link status                                       (RO/LL)
#define STE100P_XSR_JABBER       Bit(1)    // Jabber condition is detected (10Base-T only)      (RO/LH)
#define STE100P_XSR_EXT          Bit(0)    // Extended register support, always 1 for STE100P   (RO)


// STE100P PHY identification  bit defines.

#define	STE100P_PID1_PHYID_VAL   0x1C04  // (RO)

#define	STE100P_PID2_PHYID_MASK  0xFC00  // (RO)
#define	STE100P_PID2_PHYID_VAL   0x0
#define	STE100P_PID2_MODEL_MASK   0x01F0  // (RO)
#define	STE100P_PID2_MODEL_VAL    0x1
#define	STE100P_PID2_REV_MASK     0x000F  // (RO)
#define	STE100P_PID2_REV_VAL      0x1

// STE100P auto-negatiation  bit defines.

#define	STE100P_ANA_NXTPG          Bit(15)  //Next Page avability, always 0 for STE100P.  (RO)
#define	STE100P_ANA_RF             Bit(13) // Remote fault function. (RW)
#define	STE100P_ANA_FC             Bit(10) // Flow Control function Ability, 1 - supports PAUSE operation of flow control for full-duplex link. (RW)
#define	STE100P_ANA_T4             Bit(9)  // 100Base-T4 ability, always 0 for STE100P. (RO)
#define	STE100P_ANA_TXF            Bit(8)  // 100Base-TX full duplex ability, 1 with 100Base-TX full duplex ability. (RW)
#define	STE100P_ANA_TXH            Bit(7)  // 100Base-TX half duplex ability, 1 with 100Base-TX half duplex ability. (RW)

#define	STE100P_ANA_10F            Bit(6)  // 10Base-T full duplex ability, 1 with 10Base-T full duplex ability. (RW)
#define	STE100P_ANA_10H            Bit(5)  // 10Base-T half duplex ability, 1 with 10Base-T half duplex ability. (RW)
#define	STE100P_ANA_SF             0x0000  // select field, default val 0, (RO)
#define	STE100P_ANA_SF_MASK        0x000F  // STE100P_ANA_SF mask

// STE100P auto-negatiation link partner ability  bit defines.

#define	STE100P_ANLP_LPNP          Bit(15) // Link partner next page, 1 ON.  (RO)
#define	STE100P_ANLP_LPACK         Bit(14) // Received link parnter ACK, 1 received.  (RO)
#define	STE100P_ANLP_LPRF          Bit(13) // Link partner's remote fault status, 1 - remote fault detected. (RO)
#define	STE100P_ANLP_LPFC          Bit(10) // Link partner's flow control ability, 1 - link partner with PAUSE function full duplex link ability. (RO)
#define	STE100P_ANLP_LPT4          Bit(9)  // Link partner's 100Base-T4 ability, 1 - link parnter with 100Base-T4 ability. (RO)
#define	STE100P_ANLP_LPTXF         Bit(8)  // Link partner's 100Base-TX full duplex ability, 1 - link parnter with 100Base-TX full duplex ability. (RO)
#define	STE100P_ANLP_LPTXH         Bit(7)  // Link partner's 100Base-TX half duplex ability, 1 - link parnter with 100Base-TX half duplex ability. (RO)
#define	STE100P_ANLP_LP10F         Bit(6)  // Link partner's 10Base-T full duplex ability, 1 - link parnter with 10Base-T full duplex ability. (RO)
#define	STE100P_ANLP_LP10H         Bit(5)  // Link partner's 10Base-T half duplex ability, 1 - link parnter with 10Base-T half duplex ability. (RO)
#define	STE100P_ANLP_LPFS          0x001F  // Link partner selct field, default 0x0001 = IEEE 802.3 (RO)

// STE100P auto-negatiation expansion bit defines.

#define	STE100P_ANE_PDF            Bit(4) // Parallel detection fault, 1 - fault detecrted (RO/LH).
#define	STE100P_ANE_LPNP           Bit(3) // Link partner's next page ability, 1 - link partner with next page ability (RO).
#define	STE100P_ANE_NP             Bit(2) // STE100P next page ability, always 0 (RO).
#define	STE100P_ANE_PR             Bit(1) // Page received, 1 - a new page has been received (RO/LH).
#define	STE100P_ANE_LPAN           Bit(0) // Link partner auto-negotiation ability, 1 - LP has auto-negotiation ability (RO)

// STE100P configuration information and interrupt status bit defines

#define	STE100P_XCIIS_SPEED        Bit(9) // Configured information of SPEED, 0 - 10Mbit/s, 1 - 100Mbit/s. (RO)
#define	STE100P_XCIIS_DUPLEX       Bit(8) // Configured information of DUPLEX, 0 - half-duplex, 1 - full-duplex. (RO)
#define	STE100P_XCIIS_PAUSE        Bit(7) // Configured information of PAUSE, 0 - pause function disabled, 1 - pause function enabled. (RO)
#define	STE100P_XCIIS_ANC          Bit(6) // Interrupt source of auto-negotiation completed, 0 - auto-negotiation is not completed yet. (RO/LH)
#define	STE100P_XCIIS_RFD          Bit(5) // Interrupt source of remote fault detected, 0 -fault not detected, 1 - fault detected. (RO/LH)
#define	STE100P_XCIIS_LS           Bit(4) // Interrupt source link fail, 0 - link test status is up, 1 - link is down. (RO/LH).
#define	STE100P_XCIIS_ANAR         Bit(3) // Interrupt source of auto-negotiation ACK received, 0 - auto-negotiation not received. (RO/LH)
#define	STE100P_XCIIS_PDF          Bit(2) // Interrupt source of parallel detection fault, 0 - fault not detected. (RO/LH)
#define	STE100P_XCIIS_ANPR         Bit(1) // Interrupt source of auto-negotiation page received, 1 - auto-negotiation page os received (RO/LH)
#define	STE100P_XCIIS_REF          Bit(0) // Interrupt source of receive full error, 0 - the receive error number is less than 64, 1 - 64 error packets
                                          // are received (RO/LH)

// STE100P interrupt enable register bit defines

#define	STE100P_XIE_ANCE           Bit(6) // Auto-negtiation completed interrupt enable: 0/1 - disable/enable auto-negotiation completed interrupt. (RW)
#define	STE100P_XIE_RFE            Bit(5) // Remote fault detected interrupt enable: 0/1 - disable/enable remote fault detection interrupt. (RW)
#define	STE100P_XIE_LDE            Bit(4) // Link down interrupt enable: 0/1 - disable/enable link down detection interrupt. (RW)
#define	STE100P_XIE_ANAE           Bit(3) // Auto-negotiation acknowledge interrupt enable: 0/1 - disable/enable link partner acknowledge interrupt. (RW)
#define	STE100P_XIE_PDFE           Bit(2) // Parallel detection fault interrupt enable: 0/1 - disable/enable fault parallel detection interrupt. (RW)
#define	STE100P_XIE_ANPE           Bit(1) // Auto-negotiation page received interrupt enable: 0/1 - disable/enable auto-negotiation page received interrupt. (RW)
#define	STE100P_XIE_REFE           Bit(0) // RX_ERR page full interrupt enable: 0 - disable rx_err full interrupt, 1 - enable more than 64 time rx_err interrupt (RW)

// STE100P 100Base-TX control register bit defines

#define	STE100P_100CTR_DISRER      Bit(13) // Disable the RX_ERR counter, 0 - receive error counter RX_ERR is enabled, 1 - ... disabled. (RW)
#define	STE100P_100CTR_ANC         Bit(12) // Auto-negotiation completed, 0 - auto-negotiation process has not completed yet, 1 - auto-negotiation process has completed. (RO)
#define	STE100P_100CTR_ENRLB       Bit(9)  // Enable remote loop-back function: 1 - enable, 0 - disable (RW)
#define	STE100P_100CTR_ENDCR       Bit(8)  // Enable DC restoration: 0 - disable, 1 - enable (RW)
#define	STE100P_100CTR_ENRZI       Bit(7)  // Enable the conversions between NRZ and NRZI (RW)
#define	STE100P_100CTR_EN4B5B      Bit(6)  // Enable 4B/5B encoder and decoder, 0 - the 4B/5B encoder and decoder are bypassed, 1 - .. enabled. (RW)
#define	STE100P_100CTR_ISOTX       Bit(5)  // Transmit isolation: 1 - isolate from MII, 0 - for normal operation. (RW)
#define	STE100P_100CTR_CMODE_MASK  0x001C  // Reporting of current operation mode of transceiver:                 (RO)
                                           // 000 - in auto-negotiation
                                           // 001 - 10Base-T half duplex
                                           // 010 - 100Base-TX half duplex
                                           // 011 - reserved
                                           // 100 - reserved
                                           // 101 - 10Base-T full duplex
                                           // 110 - 100Base-TX full duplex
                                           // 111 - isolation, auto-negotiation disable
#define	STE100P_100CTR_DISMLT      Bit(1)  // Disable MLT3, 0 - encoder/decoder enabled, 1 - ... disabled, (RW)
#define	STE100P_100CTR_DISCRM      Bit(0)  // Disable Scramble, 0 - scrambler/descambler enabled, 1 - scrambler/descambler disabled, (RW)


// STE100P mode control register bit defines

#define	STE100P_XMC_LD            Bit(11) // Long distance mode of 10Base-T: 0 - notmal squelch level, 1 - reduces squelch for extended cable lenght. (RW)
#define	STE100P_XMC_PAD_MASK      0x00F8  // PHY addres 4:0 - 0x0000 after reset causes to isolate the PHY from MII (PR0 - 10 bit is set) (RW).
#define	STE100P_XMC_MFPSE         Bit(1)  // MF preamble supression enable, 1 - accept management frames with pre-amble supressed. (RW)


/*   (R0) - register is rea-only.
 *   (RW) - register is rea-write.
 *   (LH) - latching high and cleared by reading.
 *   (LL) - latching low and cleared by reading.
 */

unsigned int ste100p_IsPhyConnected (AT91PS_EMAC p_mac);
unsigned int ste100p_Isolate (AT91PS_EMAC p_mac);
void ste100p_DisableInterrupts (AT91PS_EMAC p_mac);
UCHAR ste100p_GetLinkSpeed (AT91PS_EMAC p_mac);
UCHAR ste100p_InitPhy (AT91PS_EMAC p_mac);
UCHAR ste100p_AutoNegotiate (AT91PS_EMAC p_mac, int *status);

#endif // _STE100P_H_

