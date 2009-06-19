//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : main.c
//* Object              :
//* Creation            : HIi   10/10/2003
//*----------------------------------------------------------------------------
#include <AT91RM9200.h>
#include <lib_AT91RM9200.h>

#include "com.h"
#include "main.h"
#include "dataflash.h"
#include "sdram.h"

#define AT91C_UBOOT_ADDR 0x21F00000
#define AT91C_UBOOT_SIZE 128*1024
#define AT91C_UBOOT_DATAFLASH_ADDR 0x00008000

#define DISP_LINE_LEN 16

//* prototypes
extern void AT91F_DBGU_Printk(char *);
extern "C" void AT91F_ST_ASM_Handler(void);

const char *menu_separ = "\n\r*----------------------------------------*\n\r";

const char *menu_dataflash =
{
	"1. Download data to dataflash [addr]\n\r"
	"2. Read data from memory [addr]\n\r"
	"3. Read data from dataflash [addr]\n\r"
	"4. Start U-BOOT\n\r"
	"5. Clear bootloader section in Dataflash\n\r"
	// "6. Memory test\n\r"
	"6. Memory info\n\r"
	"7. Bootloader info"
};

const char *menu_memory_test =
{
	"1. March test\n\r"
	"2. Address test"
	//"3. GALloping PATtern (ping-pong) test [start address]"
};

//* Globales variables
char message[40];
volatile char XmodemComplete = 0;
unsigned int StTick;

AT91S_RomBoot const *pAT91;
AT91S_SBuffer   	sXmBuffer;
AT91S_SvcXmodem 	svcXmodem;
AT91S_Pipe      	xmodemPipe;
AT91S_CtlTempo  	ctlTempo;
AT91S_SvcTempo 		svcTempo; 	 // Link to a AT91S_Tempo object


//*--------------------------------------------------------------------------------------
//* Function Name       : GetTickCount()
//* Object              : Return the number of systimer tick
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
unsigned int GetTickCount(void)
{
	return StTick;
}


//*--------------------------------------------------------------------------------------
//* Function Name       : AT91_XmodemComplete()
//* Object              : Perform the remap and jump to appli in RAM
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
void AT91_XmodemComplete(AT91S_PipeStatus status, void *pVoid)
{
	// stop the Xmodem tempo
	svcXmodem.tempo.Stop(&(svcXmodem.tempo));
	XmodemComplete = 1;
}


//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_XmodemProtocol(AT91S_PipeStatus status, void *pVoid)
//* Object              : Xmodem dispatcher
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
void XmodemProtocol(AT91S_PipeStatus status, void *pVoid)
{
	AT91PS_SBuffer pSBuffer = (AT91PS_SBuffer) xmodemPipe.pBuffer->pChild;
	AT91PS_USART   pUsart     = svcXmodem.pUsart;

	if (pSBuffer->szRdBuffer == 0)
	{
		// Start a tempo to wait the Xmodem protocol complete
		svcXmodem.tempo.Start(&(svcXmodem.tempo), 10, 0, AT91_XmodemComplete, pUsart);
	}
}


//*-------------------------- Interrupt handlers ----------------------------------------
//*--------------------------------------------------------------------------------------
//* Function Name       : irq1_c_handler()
//* Object              : C Interrupt handler for Interrutp source 1
//* Input Parameters    : none
//* Output Parameters   : none
//*--------------------------------------------------------------------------------------
extern "C" void AT91F_ST_Handler(void);

void AT91F_ST_Handler(void)
{
	volatile unsigned int csr = *AT91C_DBGU_CSR;
	unsigned int error;

	/* ========== Systimer interrupt ============== */
	if (AT91C_BASE_ST->ST_SR & 0x01)
	{
		StTick++;
		ctlTempo.CtlTempoTick(&ctlTempo);
		return;
	}

	error = AT91F_US_Error((AT91PS_USART)AT91C_BASE_DBGU);
	if (csr & error)
	{
		// Stop previous Xmodem transmition
		*(AT91C_DBGU_CR) = AT91C_US_RSTSTA;
		AT91F_US_DisableIt((AT91PS_USART)AT91C_BASE_DBGU, AT91C_US_ENDRX);
		AT91F_US_EnableIt((AT91PS_USART)AT91C_BASE_DBGU, AT91C_US_RXRDY);
	}
	else if (csr & (AT91C_US_TXRDY | AT91C_US_ENDTX | AT91C_US_TXEMPTY |
		AT91C_US_RXRDY | AT91C_US_ENDRX | AT91C_US_TIMEOUT |
		AT91C_US_RXBUFF))
	{
		if (!(svcXmodem.eot))
			svcXmodem.Handler(&svcXmodem, csr);
	}
}


//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_DisplayMenu()
//* Object              :
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
void AT91F_DisplayMenu(void)
{
	printf("\n\r");
	printf(menu_separ);
	printf("%s %s", AT91C_NAME, AT91C_VERSION);
	printf(menu_separ);
	printf(menu_dataflash);
	printf(menu_separ);
}

//*-----------------------------------------------------------------------------
//* Function Name       : AsciiToHex()
//* Object              : ascii to hexa conversion
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
unsigned int AsciiToHex(char *s, unsigned int *val)
{
	int n;

	*val=0;

	if(s[0] == '0' && ((s[1] == 'x') || (s[1] == 'X')))
		s+=2;
	n = 0;

	while((n < 8) && (s[n] !=0))
    {
		*val <<= 4;
		if ( (s[n] >= '0') && (s[n] <='9'))
			*val += (s[n] - '0');
		else if ((s[n] >= 'a') && (s[n] <='f'))
			*val += (s[n] - 0x57);
		else if ((s[n] >= 'A') && (s[n] <='F'))
			*val += (s[n] - 0x37);
		else
			return 0;
		n++;
    }

	return 1;
}

//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_MemoryDisplay()
//* Object              : Display the content of the memory
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
int AT91F_MemoryDisplay(unsigned int addr, unsigned int size, unsigned int length, bool is_dataflash)
{
	unsigned long i, nbytes, linebytes;
	char *cp;
	unsigned int 	*uip;
	unsigned short *usp;
	unsigned char *ucp;
	char linebuf[DISP_LINE_LEN];

	nbytes = length * size;
	do
	{
		uip = (unsigned int *)linebuf;
		usp = (unsigned short *)linebuf;
		ucp = (unsigned char *)linebuf;

		printf("%08x:", addr);
		linebytes = (nbytes > DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

		if (is_dataflash == true)
		{
			read_dataflash(addr, (linebytes/size)*size, linebuf);
		}
		else
		{
			for (i = 0; i < (linebytes/size)*size; i++)
			{
				linebuf[i] = *((char *)(addr + i));
			}
		}

		for (i=0; i < linebytes; i+= size)
		{
			if (size == 4)
				printf(" %08x", *uip++);
			else if (size == 2)
				printf(" %04x", *usp++);
			else
				printf(" %02x", *ucp++);
			addr += size;
		}
		printf("    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++)
		{
			if ((*cp < 0x20) || (*cp > 0x7e))
				printf("..");
			else
				printf("%c ", *cp);
			cp++;
		}
		printf("\n\r");
		nbytes -= linebytes;
	}
	while (nbytes > 0);

	return 0;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_ResetRegisters
//* Object              : Restore the initial state to registers
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
void AT91F_ResetRegisters(void)
{
	volatile int i = 0;

	AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

	/* close all peripheral clocks */
	AT91C_BASE_PMC->PMC_PCDR = 0xFFFFFFFC;

	//* Disable core interrupts and set supervisor mode
	__asm__ ("msr CPSR_c, #0xDF"); //* ARM_MODE_SYS(0x1F) | I_BIT(0x80) | F_BIT(0x40)

	//* Clear all the interrupts
	*AT91C_AIC_ICCR = 0xffffffff;

	/* read the AIC_IVR and AIC_FVR */
	i = *AT91C_AIC_IVR;
	i = *AT91C_AIC_FVR;

	/* write the end of interrupt control register */
	*AT91C_AIC_EOICR	= 0;
}

void AT91F_StartUboot(unsigned int dummy, void *pvoid)
{
	printf("Load U-BOOT from dataflash[0x%x] to SDRAM[0x%x]\n\r", AT91C_UBOOT_DATAFLASH_ADDR, AT91C_UBOOT_ADDR);
	read_dataflash(AT91C_UBOOT_DATAFLASH_ADDR, AT91C_UBOOT_SIZE, (char *)(AT91C_UBOOT_ADDR));
	printf("Start U-BOOT...\n\r");

	AT91F_ResetRegisters();
	__asm__("ldr pc, =0x21f00000");
}

//*----------------------------------------------------------------------------
//* Function Name       : main
//* Object              : Main function
//* Input Parameters    : none
//* Output Parameters   : True
//*----------------------------------------------------------------------------
int main(void)
{
	AT91PS_Buffer  		pXmBuffer;
	AT91PS_SvcComm 		pSvcXmodem;
	AT91S_SvcTempo 		svcUbootTempo; 	 // Link to a AT91S_Tempo object

	unsigned int download_address, download_size;
	unsigned int arg = 0;
	volatile int i = 0;
	char command = 0;
	unsigned int crc1 = 0, crc2 = 0;
	volatile int device;
	int NbPage;

	stdin = fopen(0, at91_dbgu_getc);
	stdout = fopen(at91_dbgu_putc, 0);

	pAT91 = AT91C_ROM_BOOT_ADDRESS;

	// Tempo Initialisation
	pAT91->OpenCtlTempo(&ctlTempo, (void *) &(pAT91->SYSTIMER_DESC));
	ctlTempo.CtlTempoStart((void *) &(pAT91->SYSTIMER_DESC));

	// Attach the tempo to a tempo controler
	ctlTempo.CtlTempoCreate(&ctlTempo, &svcUbootTempo);

	//	Xmodem Initialisation
	pXmBuffer     = pAT91->OpenSBuffer(&sXmBuffer);
	pSvcXmodem = pAT91->OpenSvcXmodem(&svcXmodem, (AT91PS_USART)AT91C_BASE_DBGU, &ctlTempo);
	pAT91->OpenPipe(&xmodemPipe, pSvcXmodem, pXmBuffer);

	//* System Timer initialization
	AT91F_AIC_ConfigureIt (
		 AT91C_BASE_AIC,                        // AIC base address
		 AT91C_ID_SYS,                          // System peripheral ID
		 AT91C_AIC_PRIOR_HIGHEST,               // Max priority
		 AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, // Level sensitive
		 AT91F_ST_ASM_Handler );
	//* Enable ST interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);

	//	DataFlash on SPI Configuration
	AT91F_DataflashInit ();

	// start tempo to start Uboot in a delay of 1 sec if no key pressed
	svcUbootTempo.Start(&svcUbootTempo, 1000, 0, AT91F_StartUboot, (void *)0);

	printf("press any key to enter bootloader... ");
	getc();

	// stop tempo
	svcUbootTempo.Stop(&svcUbootTempo);

	while(1)
    {
		while(command == 0)
		{
			download_address = AT91C_DOWNLOAD_BASE_ADDRESS;
			download_size = AT91C_DOWNLOAD_MAX_SIZE;
			arg = 0;

			AT91F_DisplayMenu();
			message[0] = 0;
			message[2] = 0;
			AT91F_ReadLine("Enter: ", message);

			command = message[0];
			if(command == '1' || command == '2' || command == '3')
				if(AsciiToHex(&message[2], &arg) == 0)
					command = 0;

			switch(command)
			{
				case '1':
					printf("Please send data by xmodem... ");
					break;

				case '2':
					do
					{
						AT91F_MemoryDisplay(arg, 4, 64, false);
						AT91F_ReadLine ((char *)0, message);
						arg += 0x100;
					}
					while(message[0] == '\0');
					command = 0;
					break;

				case '3':
					do
					{
						AT91F_MemoryDisplay(arg, 4, 64, true);
						AT91F_ReadLine ((char *)0, message);
						arg += 0x100;
					}
					while(message[0] == '\0');
					command = 0;
					break;

				case '4':
					AT91F_StartUboot(0, (void *)0);
					command = 0;
					break;

				// clear bootsector
				case '5':
					int *i;
					AT91F_ReadLine ("\n\rAre you sure you want to erase LAB bootloader from dataflash? (y/n) ",
							message);
					if(message[0] == 'Y' || message[0] == 'y')
					{
						for(i = (int *)AT91C_SDRAM_BASE_ADDRESS; i < (int *)(AT91C_SDRAM_BASE_ADDRESS + AT91C_ISRAM_SIZE); i++)
							*i = 0;
						write_dataflash(0x00000000, AT91C_SDRAM_BASE_ADDRESS, AT91C_ISRAM_SIZE);
						printf("Bootsector cleared\n\r");
						AT91F_WaitKeyPressed();
					}
					else
					{
						printf("Erase aborted\n\r");
						AT91F_WaitKeyPressed();
					}

					command = 0;
					break;

				/*
				 case '6':
					printf(menu_separ);
					printf(menu_memory_test);
					printf(menu_separ);

					message[0] = 0;
					message[2] = 0;

					AT91F_ReadLine("Enter: ", message);
					command = message[0];

					switch(command)
					{
						case '1':
							AT91F_MarchTest();
							AT91F_WaitKeyPressed();
							break;

						case '2':
							AT91F_AddressTest();
							AT91F_WaitKeyPressed();
							break;

						case '3':
							if(AsciiToHex(&message[2], &arg) == 0)
								break;
							AT91F_GallopingPatternTest(arg);
							AT91F_WaitKeyPressed();
							break;

						default:
							break;
					}

					command = 0;
					break;
				*/

				// Dataflash and SDRAM info
				case '6':
					printf(menu_separ);
					printf("SDRAM memory: %dMB @ %dMHz\n\r\n\r",
						AT91C_SDRAM_SIZE/(1024*1024),
						AT91C_MASTER_CLOCK/1000000);

					AT91F_DataflashPrintInfo();
					printf(menu_separ);
					AT91F_WaitKeyPressed();
					command = 0;
					break;

				// Bootloader info
				case '7':
					printf(menu_separ);
					printf("%s %s\n\r", AT91C_NAME, AT91C_VERSION);
					printf("Based on AT91RM9200-Loader gcc port\n\r");
					printf("(http://www.open-research.org.uk/ARMuC/At91rm9200_Booting.html)\n\r");
					printf("Modified by Kuba Odias, %s, %s", __DATE__, __TIME__);
					printf(menu_separ);
					AT91F_WaitKeyPressed();
					command = 0;
					break;

				default:
					command = 0;
					break;
			}
		}
		XmodemComplete = 0;
		xmodemPipe.Read(&xmodemPipe, (char *)download_address, download_size, XmodemProtocol, 0);
		while(XmodemComplete !=1);
		download_size = (unsigned int)(svcXmodem.pData) - (unsigned int)download_address;
		printf("\n\rDownloaded %d bytes to [0x%x]\n\r", download_size, download_address);

		// ask user if really write dataflash
		printf("Are you sure you want to write dataflash at [0x%x]? (y/n) ", arg);
		AT91F_ReadLine ("", message);
		if ((message[0] == 'Y' || message[0] == 'y') && (arg == 0x00))
		{
			AT91F_ReadLine ("DANGER!!! This will overwrite your 1st bootloader!!! Continue? (y/n) ", message);
		}
		if (message[0] == 'Y' || message[0] == 'y')
		{
			// Modification of vector 6
			NbPage = 0;
			i = dataflash_info.Device.pages_number;
			while(i >>= 1)
				NbPage++;
			i = (download_size / 512) + 1 + (NbPage << 13) + (dataflash_info.Device.pages_size << 17);
			*(int *)(download_address + AT91C_OFFSET_VECT6) = i;
			printf("\n\rModification of Arm Vector 6 :%x\n\r", i);

			printf("\n\rWrite %d bytes in DataFlash [0x%x]\n\r",download_size, arg);
			crc1 = 0;
			pAT91->CRC32((const unsigned char *)download_address, download_size , &crc1);

			// write the dataflash
			write_dataflash (arg, download_address, download_size);
			// clear the buffer before read
			for(i=0; i < download_size; i++)
				*(unsigned char *)(download_address + i) = 0;

			//* Read dataflash page in TestBuffer
			read_dataflash (arg, download_size, (char *)(download_address));

			printf("Verify Dataflash: ");
			crc2 = 0;

			pAT91->CRC32((const unsigned char *)download_address, download_size , &crc2);
			if (crc1 != crc2)
				printf("Failed\n\r");
			else
				printf("OK\n\r");

			AT91F_WaitKeyPressed();
		}
		command = 0;
		XmodemComplete = 0;
    }
}
