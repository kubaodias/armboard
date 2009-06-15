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

#define DISP_LINE_LEN 16

//* prototypes
extern void AT91F_DBGU_Printk(char *);
extern "C" void AT91F_ST_ASM_Handler(void);

const char *menu_separ = "*----------------------------------------*\n\r";

const char *menu_dataflash =
{
	"1. Read data from memory [addr]\n\r"
	"2. Clear memory\n\r"
	"3. GALloping PATtern (ping-pong) test\n\r"
	"7. Address test\n\r"
	"9. Memory test info\n\r"
};

//* Globales variables
char message[40];

AT91S_RomBoot const *pAT91;

//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_DisplayMenu()
//* Object              :
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
void AT91F_DisplayMenu(void)
{
	printf("\n\r\n\r");
	printf(menu_separ);
	printf("%s %s\n\r", AT91C_NAME, AT91C_VERSION);
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
int AT91F_MemoryDisplay(unsigned int addr, unsigned int size, unsigned int length)
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

		for (i = 0; i < (linebytes/size)*size; i++)
		{
			linebuf[i] = *((char *)(addr + i));
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
void AT91F_GallopingPatternTest()
{
	unsigned int test_addr, addr, val, readback;
	unsigned int start_addr = AT91C_SDRAM_BASE_ADDRESS;
	unsigned int end_addr = AT91C_SDRAM_BASE_ADDRESS + AT91C_SDRAM_SIZE;
	unsigned char test_result = 0;

	printf("\n\rPerforming memory GALloping PATtern test");
	AT91F_ClearSDRAM();
	for (test_addr = start_addr; test_addr < end_addr; test_addr += 4)
	{
		// complement test cell
		val = *(unsigned int *)test_addr;
		*(unsigned int *)test_addr = ~val;

		for (addr = start_addr; addr < end_addr; addr += 4)
		{
			// read each memory cell
			if (addr == test_addr)
			{
				readback = *(unsigned int *)addr;
				if (readback != ~val)
				{
					if (test_result == 0)
					{
						printf("\n\rFAILED:");
					}
					printf ("\n\r  !! Memory error at 0x%x: "
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
						printf("\n\rFAILED:");
					}
					printf ("\n\r  !! Memory error at 0x%x: "
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
					printf("\n\rFAILED:");
				}
				printf ("\n\r  !! Memory error at 0x%x: "
					"found 0x%x, expected 0x%x !!!",
					addr, readback, ~val);
				test_result = 1;
			}
		}

		// complement test cell
		val = *(unsigned int *)test_addr;
		*(unsigned int *)test_addr = ~val;

		if (test_result == 0)
			printf(" .");
	}

	if (test_result == 0)
	{
		printf("\n\rOK\n\r");
	}
	else
		printf("\n\r");
}

/* Address Test writes a unique value into each memory location.
 * Typically, this could be the address of that memory cell;
 * that is, the value n is written into memory location n.
 * After writing all memory locations, the data are read back.
 * The purpose of this test is to check for address uniqueness.
 * This algorithm requires that the number of bits in each memory
 * word equal or exceed the number of address bits.
 */
void AT91F_AddressTest()
{
	int i;
	unsigned int addr, val, incr, readback;
	unsigned int start_addr = AT91C_SDRAM_BASE_ADDRESS;
	unsigned int end_addr = AT91C_SDRAM_BASE_ADDRESS + AT91C_SDRAM_SIZE;
	char test_result = 0;

	printf("\n\rPerforming memory address test");
	test_result = 0;

	for (addr = start_addr, val = 0; addr < end_addr; addr += 4)
	{
		*(unsigned int *)addr = val;
		val++;
	}

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
			printf ("\n\r  !!! Memory error at 0x%x: "
				"found 0x%x, expected 0x%x !!!",
				addr, readback, val);
			test_result = 1;
		}
		val++;
	}

	if (test_result == 0)
	{
		printf(": OK\n\r");
	}
	else
		printf("\n\r");
}


//*----------------------------------------------------------------------------
//* Function Name       : main
//* Object              : Main function
//* Input Parameters    : none
//* Output Parameters   : True
//*----------------------------------------------------------------------------
int main(void)
{
	unsigned int arg = 0;
	volatile int i = 0;
	char command = 0;
	unsigned int crc1 = 0, crc2 = 0;

	stdin = fopen(0, at91_dbgu_getc);
	stdout = fopen(at91_dbgu_putc, 0);

	pAT91 = AT91C_ROM_BOOT_ADDRESS;

	while(1)
    {
		arg = 0;

		AT91F_DisplayMenu();
		message[0] = 0;
		message[2] = 0;
		AT91F_ReadLine("Enter: ", message);

		command = message[0];
		if(command == '1')
			if(AsciiToHex(&message[2], &arg) == 0)
				command = 0;

		switch(command)
		{
			case '1':
				do
				{
					AT91F_MemoryDisplay(arg, 4, 64);
					AT91F_ReadLine ((char *)0, message);
					arg += 0x100;
				}
				while(message[0] == '\0');
				break;

			case '2':
				AT91F_ClearSDRAM();
				printf("\n\rMemory successfully cleared.\n\r");
				AT91F_WaitKeyPressed();
				break;

			case '3':
				AT91F_GallopingPatternTest();
				AT91F_WaitKeyPressed();
				break;

			case '7':
				AT91F_AddressTest();
				AT91F_WaitKeyPressed();
				break;

			// memory test info
			case '9':
				printf("\n\r");
				printf(menu_separ);
				printf("%s %s\n\r", AT91C_NAME, AT91C_VERSION);
				printf("Memory tests for %dMB of SDRAM @ %dMHz\n\r",
						AT91C_SDRAM_SIZE/(1024*1024),
						AT91C_MASTER_CLOCK/1000000);
				printf("Written by Kuba Odias, %s, %s\n\r", __DATE__, __TIME__);
				printf(menu_separ);
				AT91F_WaitKeyPressed();
				break;

			default:
				break;
		}
    }
}
