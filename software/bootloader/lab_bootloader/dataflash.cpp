//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : dataflash.c
//* Object              : High level functions for the dataflash
//* Creation            : HIi   10/10/2003
//*----------------------------------------------------------------------------

#include "dataflash.h"
#include "com.h"

AT91S_DATAFLASH_INFO dataflash_info;
static AT91S_DataFlash DataFlashInst;

int AT91F_DataflashInit(void)
{
	int dfcode;

	AT91F_SpiInit();

	dataflash_info.Desc.state = IDLE;
	dataflash_info.id = 0;
	dataflash_info.Device.pages_number = 0;
	dfcode = AT91F_DataflashProbe(&dataflash_info.Desc);

	// only AT45DB041 is allowed
	switch (dfcode)
	{
		case AT45DB041:
			dataflash_info.Device.pages_number = 2048;
			dataflash_info.Device.pages_size = 264;
			dataflash_info.Device.page_offset = 9;
			dataflash_info.Device.byte_mask = 0x100;
			dataflash_info.Desc.DataFlash_state = IDLE;
			dataflash_info.id = dfcode;
			break;
		default:
			break;
	}

	return 1;
}


void AT91F_DataflashPrintInfo(void)
{
	printf("Dataflash info:\n\r");
	switch (dataflash_info.id)
	{
		case AT45DB041:
			printf ("   Device:            AT45DB041\n\r");
			printf ("   Number of pages:   %d\n\r"
					"   Page size:         %d",
					(unsigned int) dataflash_info.Device.pages_number,
					(unsigned int) dataflash_info.Device.pages_size);
			break;
		default:
			printf("   Valid dataflash device wasn't found");
	}
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataflashSelect 									*/
/* Object              : Select the correct device								*/
/*------------------------------------------------------------------------------*/
AT91PS_DataFlash AT91F_DataflashSelect (AT91PS_DataFlash pFlash)
{
	pFlash->pDataFlashDesc = &(dataflash_info.Desc);
	pFlash->pDevice = &(dataflash_info.Device);
	return (pFlash);
}

/*------------------------------------------------------------------------------*/
/* Function Name       : read_dataflash 					*/
/* Object              : dataflash memory read					*/
/*------------------------------------------------------------------------------*/
int read_dataflash(unsigned long addr, unsigned long size, char *result)
{
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect(pFlash);
	if (pFlash == 0)
		return -1;

	return (AT91F_DataFlashRead(pFlash, addr, size, result));
}


/*-----------------------------------------------------------------------------*/
/* Function Name       : write_dataflash 				       */
/* Object              : write a block in dataflash			       */
/*-----------------------------------------------------------------------------*/
int write_dataflash (unsigned long addr_dst, unsigned int addr_src,
		     unsigned int size)
{
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect(pFlash);
	if (pFlash == 0)
		return -1;

	return AT91F_DataFlashWrite(pFlash, (unsigned char *) addr_src, addr_dst, size);
}
