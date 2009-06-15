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
	"8. Memory test info\n\r"
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

void memory_test(unsigned int pattern_number)
{
	static const unsigned int bitpattern[] =
	{
			/*0x00000001,	/* single bit */
			/*0x00000003,	/* two adjacent bits */
			/*0x00000007,	/* three adjacent bits */
			/*0x0000000F,	/* four adjacent bits */
			/*0x00000005,	/* two non-adjacent bits */
			/*0x00000015,	/* three non-adjacent bits */
			/*0x00000055,	/* four non-adjacent bits */
			/*0xaaaaaaaa,	/* alternating 1/0 */
			0x00000000,
			0x11111111,
			0x33333333,
			0x55555555,
			0x0f0f0f0f,
			0x00ff00ff,
	};
	int i;
	unsigned int pattern, addr, val, incr, readback;
	unsigned int start_addr = AT91C_SDRAM_BASE_ADDRESS;
	unsigned int end_addr = AT91C_SDRAM_BASE_ADDRESS + AT91C_SDRAM_SIZE;
	char test_result;

	int pattern_count = sizeof(bitpattern) / sizeof(unsigned int);

	if (pattern_number > pattern_count)
	{
		printf("Unsupported test %d\n\r", pattern);
		return;
	}

	if (pattern_number == 0)
	{
		printf("Performing full memory test\n\r");
		for (i = 1; i <= pattern_count; i++)
		{
			memory_test(i);
		}

		// return from test
		return;
	}

	// select proper bit pattern
	pattern = bitpattern[pattern_number - 1];

	printf ("* Memory test number %d: \n\r", pattern_number);
	incr = 1;

	// for each pattern do 2 tests with different variations
	for (i = 0; i < 2; i++)
	{
		printf("  - testing pattern 0x%8x", pattern);

		test_result = 0;

		for (addr = start_addr, val = pattern; addr < end_addr; addr+=4)
		{
			*(unsigned int *)addr = val;
			val  += incr;
		}

		for (addr=start_addr, val = pattern; addr < end_addr; addr+=4)
		{
			readback = *(unsigned int *)addr;
			if (readback != val)
			{
				// if this is the first error
				if (test_result == 0)
				{
					printf(": FAILED");
				}
				printf ("\n\r    !!! Memory error at 0x%x: "
					"found 0x%x, expected 0x%x !!!",
					addr, readback, val);
				test_result = 1;
			}
			val += incr;
		}

		if (test_result == 0)
		{
			printf(": OK\n\r");
		}
		else
			printf("\n\r");

		/*
		 * Flip the pattern each time to make lots of zeros and
		 * then, the next time, lots of ones.  We decrement
		 * the "negative" patterns and increment the "positive"
		 * patterns to preserve this feature.
		 */
		if(pattern & 0x80000000) {
			pattern = -pattern;	/* complement & increment */
		}
		else {
			pattern = ~pattern;
		}
		incr = -incr;
	}
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
				command = 0;
				break;

			case '6':
				printf("\n\r");
				printf(menu_separ);
				memory_test(arg);
				printf(menu_separ);
				AT91F_WaitKeyPressed();
				command = 0;
				break;

			// memory test info
			case '8':
				printf("\n\r");
				printf(menu_separ);
				printf("%s %s\n\r", AT91C_NAME, AT91C_VERSION);
				printf("Written by Kuba Odias, %s, %s\n\r", __DATE__, __TIME__);
				printf(menu_separ);
				AT91F_WaitKeyPressed();
				break;

			default:
				break;
		}
    }
}
