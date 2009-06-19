#include <AT91RM9200.h>
#include <lib_AT91RM9200.h>
#include "com.h"

static char erase_seq[] = "\b \b";		/* erase sequence	*/
static char   tab_seq[] = "        ";		/* used to expand TABs	*/

int at91_dbgu_putc(int ch)
{
  while (!AT91F_US_TxReady((AT91PS_USART)AT91C_BASE_DBGU));
  AT91F_US_PutChar((AT91PS_USART)AT91C_BASE_DBGU, (char)ch);
  return ch;
}

int at91_dbgu_getc()
{
  while(!AT91F_US_RxReady((AT91PS_USART)AT91C_BASE_DBGU));
  return((int)AT91F_US_GetChar((AT91PS_USART)AT91C_BASE_DBGU));
}


//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_ClrScr()
//* Object              : Send a clear screen on the USART
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
/*void AT91F_ClrScr(void)
{
  puts(CLRSCREEN);
}*/


//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_DeleteChar()
//* Object              :
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
char *AT91F_DeleteChar(char *buffer, char *p, int *colp, int *np, int plen)
{
  char *s;

  if (*np == 0) {
    return (p);
  }

  if (*(--p) == '\t') {			/* will retype the whole line	*/
    while (*colp > plen) {
      puts(erase_seq);
      (*colp)--;
    }
    for (s=buffer; s<p; ++s) {
      if (*s == '\t') {
	puts(tab_seq+((*colp) & 07));
	*colp += 8 - ((*colp) & 07);
      } else {
	++(*colp);
	putc(*s);
      }
    }
  } else {
    puts(erase_seq);
    (*colp)--;
  }
  (*np)--;
  return (p);
}



//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_ReadLine()
//* Object              :
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
int AT91F_ReadLine (const char *const prompt, char *console_buffer)
{
  char *p = console_buffer;
  int	n = 0;					/* buffer index		*/
  int	plen = strlen (prompt);	                /* prompt length	*/
  int	col;					/* output column cnt	*/
  char	c;

  /* print prompt */
  if(prompt)
    puts(prompt);
  col = plen;

  for (;;)
    {
      c = getc();

      switch (c)
	{
	case '\r':				/* Enter		*/
	case '\n':
	  *p = '\0';
	  puts ("\r\n");
	  return (p - console_buffer);

	case 0x03:				/* ^C - break	*/
	  console_buffer[0] = '\0';	/* discard input */
	  return (-1);

	case 0x15:				/* ^U - erase line	*/
	  while (col > plen)
	    {
	      puts(erase_seq);
	      --col;
	    }
	  p = console_buffer;
	  n = 0;
	  continue;

	case 0x17:				/* ^W - erase word 	*/
	  p = (char *)AT91F_DeleteChar(console_buffer, p, &col, &n, plen);
	  while ((n > 0) && (*p != ' '))
	    p = (char *)AT91F_DeleteChar(console_buffer, p, &col, &n, plen);
	  continue;

	case 0x08:				/* ^H  - backspace	*/
	case 0x7F:				/* DEL - backspace	*/
	  p=(char *)AT91F_DeleteChar(console_buffer, p, &col, &n, plen);
	  continue;

	default:
	  /*
	   * Must be a normal character then
	   */
	  if (n < (AT91C_CB_SIZE -2))
	    {
	      ++col;		/* echo input		*/
	      putc(c);
	      *p++ = c;
	      ++n;
	    }
	  else 			/* Buffer full		*/
	    putc('\a');
	}
    }
}


//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_WaitKeyPressed()
//* Object              :
//* Input Parameters    :
//* Return value		:
//*-----------------------------------------------------------------------------
void AT91F_WaitKeyPressed(void)
{
	int c;
	puts("press any key...");
	c = getc();
}


