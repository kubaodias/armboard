//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : init.c
//* Object              : Low level initialisations written in C
//* Creation            : HIi   10/10/2003
//*
//*----------------------------------------------------------------------------

#include <AT91RM9200.h>
#include <lib_AT91RM9200.h>
#include "main.h"

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_Printk
//* \brief This function is used to send a string through the DBGU channel (Very low level debugging)
//*----------------------------------------------------------------------------
void AT91F_DBGU_Printk(
	char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {
		while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_DBGU));
		AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_DBGU, *buffer++);
	}
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DataAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_SpuriousHandler()
{
	AT91F_DBGU_Printk("-F- Spurious Interrupt detected\n\r");
	while (1);
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_DataAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_DataAbort()
{
	AT91F_DBGU_Printk("-F- Data Abort detected\n\r");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_FetchAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_FetchAbort()
{
	AT91F_DBGU_Printk("-F- Prefetch Abort detected\n\r");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_Undef
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
void AT91F_Undef()
{
	AT91F_DBGU_Printk("-F- Undef detected\n\r");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UndefHandler
//* \brief This function reports that no handler have been set for current IT
//*----------------------------------------------------------------------------
void AT91F_UndefHandler()
{
	AT91F_DBGU_Printk("-F- Undef detected\n\r");
	while (1);
}

//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_SetClocks
//* Object              : Set the PLLA to 180Mhz and Master clock to 60 Mhz
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
void AT91F_SetClocks(void)
{
	volatile int tmp;

	AT91PS_PMC pPmc = AT91C_BASE_PMC;
	AT91PS_CKGR pCkgr = AT91C_BASE_CKGR;

	// disable all interrupts from PMC
	pPmc->PMC_IDR = 0xFFFFFFFF;

	// disable all clocks except for processor clock
	pPmc->PMC_SCDR = 0xFFFFFFFE;

	// MCK = SLCK = processor clock
	pPmc->PMC_MCKR = AT91C_PMC_CSS_SLOW_CLK | AT91C_PMC_PRES_CLK | AT91C_PMC_MDIV_1;

	// main oscillator enable, wait 255 cycles
	pCkgr->CKGR_MOR = AT91C_CKGR_MOSCEN | AT91C_CKGR_OSCOUNT;

	// Setup the PLL A
	pCkgr->CKGR_PLLAR = AT91C_PLLA_VALUE;
	tmp = 0;
	while(!(pPmc->PMC_SR & AT91C_PMC_LOCKA) && (tmp++ < DELAY_MAIN_FREQ));

	// Setup the PLL B
	pCkgr->CKGR_PLLBR = AT91C_PLLB_VALUE;
	tmp = 0;
	while(!(pPmc->PMC_SR & AT91C_PMC_LOCKB) && (tmp++ < DELAY_MAIN_FREQ));

	// processor clock is a PLLA clock, MCK is 2 timer slower
	pPmc->PMC_MCKR = AT91C_PMC_CSS_PLLA_CLK | AT91C_PMC_PRES_CLK | AT91C_PMC_MDIV_2;

	// switch to asynchronous clocking mode
	__asm__("mrc     p15, 0, r0, c1, c0, 0");	/* read cp15 control register (cp15 r1) in r0 */
	__asm__("ldr     r3,= 0xC0000000");		/* Set bit :Asynchronous clock mode, Not Fast Bus*/
	__asm__("orr     r0, r0, r3");
	__asm__("mcr     p15, 0, r0, c1, c0, 0"); /* write r0 in cp15 control register (cp15 r1) */
}

//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_InitSdram
//* Object              : Initialize the SDRAM
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
void AT91F_InitSdram()
{
	int *pRegister;
	AT91PS_SDRC pSdrc = AT91C_BASE_SDRC;
	int i;

	//  Configure PIOC as peripheral (D16/D31)
	AT91F_PIO_CfgPeriph(
		 AT91C_BASE_PIOC, // PIO controller base address
		 0xFFFF0000,
		 0
		);

	// Init SDRAM
	// based on AT91Bootstrap for AT91SAM9260
	/* 9 columns, 13 rows, 2 CAS, 4 banks, 32 bits,
	 * 2 TWR, 7 TRC, 2 TRP, 2 TRCD, 5 TRAS, 8 TXSR
	 */
	pSdrc->SDRC_CR = 0x42913959;

	for (i =0; i< 1000;i++);

	// precharge all
	pSdrc->SDRC_MR = AT91C_SDRC_MODE_PRCGALL_CMD;

	pRegister = (int *)AT91C_SDRAM_BASE_ADDRESS;
	*pRegister = 0;

	for (i =0; i< 10000;i++);

	for (i = 0; i < 8; i++)
	{
		// refresh command
		pSdrc->SDRC_MR = AT91C_SDRC_MODE_RFSH_CMD;
		pRegister = (int *)(AT91C_SDRAM_BASE_ADDRESS + i * 4);
		*pRegister = i;
	}

	// load mode register
	pSdrc->SDRC_MR = AT91C_SDRC_MODE_LMR_CMD;

	pRegister = (int *)(AT91C_SDRAM_BASE_ADDRESS + 0x24);
	*pRegister = 0xcafedede;	// Perform LMR burst=1, lat=2

	// MCK * 7 / 1000000
	pSdrc->SDRC_TR = 0x1a4;

	// normal mode
	pSdrc->SDRC_MR = AT91C_SDRC_MODE_NORMAL_CMD;

	pRegister = (int *)AT91C_SDRAM_BASE_ADDRESS;
	*pRegister = 0;
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_InitFlash
//* \brief This function performs low level HW initialization
//*----------------------------------------------------------------------------
void AT91F_InitMemories()
{
	AT91PS_EBI pEbi = AT91C_BASE_EBI;

	// Setup MEMC to support all connected memories (CS1=SDRAM)
	pEbi->EBI_CSA = AT91C_EBI_CS1A_SDRAMC;

	AT91F_InitSdram();
}



//*----------------------------------------------------------------------------
//* \fn    AT91F_LowLevelInit
//* \brief This function performs very low level HW initialization
//*----------------------------------------------------------------------------
extern "C" void AT91F_LowLevelInit(void)
{
	// Init Interrupt Controller
	AT91F_AIC_Open(
		AT91C_BASE_AIC,          // pointer to the AIC registers
		AT91C_AIC_BRANCH_OPCODE, // IRQ exception vector
		AT91F_UndefHandler,      // FIQ exception vector
		AT91F_UndefHandler,      // AIC default handler
		AT91F_SpuriousHandler,   // AIC spurious handler
		0);                      // Protect mode

	// Perform 8 End Of Interrupt Command to make sure AIC will not Lock out nIRQ
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);
	AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);

	AT91F_AIC_SetExceptionVector((unsigned int *)0x0C, AT91F_FetchAbort);
	AT91F_AIC_SetExceptionVector((unsigned int *)0x10, AT91F_DataAbort);
	AT91F_AIC_SetExceptionVector((unsigned int *)0x4, AT91F_Undef);

	// Initialize clocks and PLL
	AT91F_SetClocks();

	// Initialize SDRAM
	AT91F_InitMemories();

	// Open PIO for DBGU
	AT91F_DBGU_CfgPIO();

	// Configure DBGU
	AT91F_US_Configure (
		(AT91PS_USART) AT91C_BASE_DBGU,          // DBGU base address
		AT91C_MASTER_CLOCK,   // 60 MHz
		AT91C_US_ASYNC_MODE,  // mode Register to be programmed
		AT91C_BAUD_RATE ,     // baudrate to be programmed
		0);                   // timeguard to be programmed

	// Enable Transmitter
	AT91F_US_EnableTx((AT91PS_USART)AT91C_BASE_DBGU);
	// Enable Receiver
	AT91F_US_EnableRx((AT91PS_USART)AT91C_BASE_DBGU);

	// AT91F_DBGU_Printk("\n\rAT91F_LowLevelInit(): Debug channel initialized\n\r");
}

