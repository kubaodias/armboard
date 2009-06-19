#include <AT91RM9200.h>
#include <lib_AT91RM9200.h>
#include "main.h"
#include "stdio.h"

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

/* Clear whole SDRAM memory */
void AT91F_ClearSDRAM()
{
	unsigned int addr = AT91C_SDRAM_BASE_ADDRESS;
	for (int i = 0; i < AT91C_SDRAM_SIZE; i += 4)
	{
		*(unsigned int *)addr = 0;
		addr += 4;
	}
}

/* A memory test pattern that tests for address nonuniqueness and other
 * functional faults in memories, as well as some dynamic faults,
 * is the GALPAT (GALloping PATtern), sometimes referred to as a ping-pong pattern.
 * This pattern accesses each address repeatedly using, at some point,
 * every other cell as a previous address. It starts by writing a background
 * of zeroes into all memory cells. Then the first cell becomes the test cell.
 * It is complemented and read alternately with every other cell in memory.
 * Each succeeding cell then becomes the test cell in turn and the entire read
 * process is repeated. All data are complemented and the entire test is repeated.
 * If each read and compare is counted as one operation, then GALPAT has an
 * execution time proportional to 4N^2, where N is the number of cells.
 * It is effective for finding cell opens, shorts, address uniqueness faults,
 * sense amplifier interaction, and access time problems.
 */
/*void AT91F_GallopingPatternTest(unsigned int start_addr)
{
	unsigned int test_addr, addr, val, readback;
	unsigned int end_addr = AT91C_SDRAM_BASE_ADDRESS + AT91C_SDRAM_SIZE;
	unsigned char test_result = 0;

	if ((start_addr < AT91C_SDRAM_BASE_ADDRESS) || (start_addr >= end_addr))
	{
		start_addr = AT91C_SDRAM_BASE_ADDRESS;
	}

	printf("\n\rPerforming memory GALloping PATtern test for 1kB area starting at 0x%8x", start_addr);
	AT91F_ClearSDRAM();
	for (test_addr = start_addr; test_addr < end_addr, test_addr - start_addr < 0x400; test_addr += 4)
	{
		printf("\n\r  * testing 0x%8x", test_addr);

		// complement test cell
		val = *(unsigned int *)test_addr;
		*(unsigned int *)test_addr = ~val;

		// check against each memory cell
		for (addr = AT91C_SDRAM_BASE_ADDRESS; addr < end_addr; addr += 4)
		{
			// test cell have unique value written in it
			if (addr == test_addr)
			{
				readback = *(unsigned int *)addr;
				if (readback != ~val)
				{
					if (test_result == 0)
					{
						printf(": FAILED");
					}
					printf ("\n\r    - Memory error at 0x%x: "
						"found 0x%x, expected 0x%x !!!",
						addr, readback, ~val);
					test_result = 1;
				}
			}
			else
			{
				readback = *(unsigned int *)addr;
				if (readback != 0)
				{
					if (test_result == 0)
					{
						printf(": FAILED");
					}
					printf ("\n\r    - Memory error at 0x%x: "
						"found 0x%x, expected 0x%x !!!",
						addr, readback, 0);
					test_result = 1;
				}
			}

			// read test cell
			readback = *(unsigned int *)test_addr;
			if (readback != ~val)
			{
				if (test_result == 0)
				{
					printf(": FAILED");
				}
				printf ("\n\r    - Memory error at 0x%x: "
					"found 0x%x, expected 0x%x !!!",
					addr, readback, ~val);
				test_result = 1;
			}
		}

		// complement test cell
		val = *(unsigned int *)test_addr;
		*(unsigned int *)test_addr = ~val;
	}

	if (test_result == 0)
	{
		printf("\n\rOK\n\r");
	}
	else
		printf("\n\rTEST FAILED\n\r");
}*/

/* March, like most of the algorithms, begins by writing a background of zeroes.
 * Then it reads the data at the first location and writes a 1 to that address.
 * It continues this read/write procedure sequentially with each address in memory.
 * When the end of memory is reached, each cell is read and changed back to zero
 * in reverse order. The test is then repeated using complemented data.
 * Execution time is of order N. It can find cell opens, shorts, address uniqueness,
 * and some cell interactions.
 */
/*void AT91F_MarchTest()
{
	unsigned int test_addr, addr, val, readback;
	unsigned int start_addr = AT91C_SDRAM_BASE_ADDRESS;
	unsigned int end_addr = AT91C_SDRAM_BASE_ADDRESS + AT91C_SDRAM_SIZE;
	unsigned char test_result = 0;

	printf("\n\rPerforming memory march test");

	AT91F_ClearSDRAM();

	val = 1;
	// repeat process 2 times
	for (int i = 0; i < 2; i++)
	{
		printf("\n\r  * writing 0x%8x", val);

		// write value to each memory cell
		for (test_addr = start_addr; test_addr < end_addr; test_addr += 4)
			*(unsigned int *)test_addr = val;

		printf("\n\r  * reading");
		// read in reverse order
		for (test_addr = end_addr - 4; test_addr >= start_addr; test_addr -= 4)
		{
			readback = *(unsigned int *)addr;

			if (readback != val)
			{
				if (test_result == 0)
				{
					printf(": FAILED");
				}
				printf ("\n\r    - Memory error at 0x%x: "
					"found 0x%x, expected 0x%x !!!",
					addr, readback, val);
				test_result = 1;
			}
		}
		// use complemented value
		val = ~val;
	}

	if (test_result == 0)
	{
		printf("\n\rOK\n\r");
	}
	else
		printf("\n\rTEST FAILED\n\r");
}*/

/* Address Test writes a unique value into each memory location.
 * Typically, this could be the address of that memory cell;
 * that is, the value n is written into memory location n.
 * After writing all memory locations, the data are read back.
 * The purpose of this test is to check for address uniqueness.
 * This algorithm requires that the number of bits in each memory
 * word equal or exceed the number of address bits.
 */
/*void AT91F_AddressTest()
{
	int i;
	unsigned int addr, val, readback;
	unsigned int start_addr = AT91C_SDRAM_BASE_ADDRESS;
	unsigned int end_addr = AT91C_SDRAM_BASE_ADDRESS + AT91C_SDRAM_SIZE;
	char test_result = 0;

	printf("\n\rPerforming memory address test");

	printf("\n\r  * writing");
	for (addr = start_addr, val = 0; addr < end_addr; addr += 4)
	{
		*(unsigned int *)addr = val;
		val++;
	}

	printf("\n\r  * reading");
	for (addr = start_addr, val = 0; addr < end_addr; addr += 4)
	{
		readback = *(unsigned int *)addr;

		if (readback != val)
		{
			// if this is the first error
			if (test_result == 0)
			{
				printf(": FAILED");
			}
			printf ("\n\r    - Memory error at 0x%x: "
				"found 0x%x, expected 0x%x !!!",
				addr, readback, val);
			test_result = 1;
		}
		val++;
	}

	if (test_result == 0)
	{
		printf("\n\rOK\n\r");
	}
	else
		printf("\n\rTEST FAILED\n\r");
}*/
