#include "stdio.h"

extern int at91_dbgu_putc(int ch);

FILE *stdout = (FILE *)0;
FILE *stdin = (FILE *)0;

FILE __filedesc[FILEDESCS];

FILE *fopen(int (*put)(int), int (*get)())
{
  static int init = 1;

  if(init != 0)
    {
      for(int i = 0; i < FILEDESCS; i++)
	{
	  __filedesc[i].active = false;
	  __filedesc[i].put = 0;
	  __filedesc[i].get = 0;
	}

      init = 0;
    }

  for(int i = 0; i < FILEDESCS; i++)
    {
      if(!__filedesc[i].active)
	{
	  __filedesc[i].put = put;
	  __filedesc[i].get = get;
	  
	  __filedesc[i].active = true;
	
	  return &__filedesc[i];
	}      
    }

  return (FILE *)0;
}
      
int fclose(FILE *fp)
{
  for(int i = 0; i < FILEDESCS; i++)
    if(&__filedesc[i] == fp || fp->active)
      {
	fp->active = false;

	fp->put = 0;
	fp->get = 0;

	return 0;
      }

  return -1;
}

int fputs(const char *str, FILE *fp)
{
  if(fp == (FILE *)0)
    return -1;

  if(fp->put == (void *)0)
    return -1;

  while(*str != 0)
    {
      fp->put(*str);
      str++;
    }
  return 0;
}

int fputc(int c, FILE *fp)
{
  if(fp == (FILE *)0)
    return -1;

  if(fp->put == (void *)0)
    return -1;

  return fp->put(c);
}

int fgetc(FILE *fp)
{
  if(fp == (FILE *)0)
    return -1;

  if(fp->get == (void *)0)
    return -1;

  return fp->get();
}


int puts(const char *str)
{
  return fputs(str, stdout);
}

int putc(int c)
{
  return fputc(c, stdout);
}

int putchar(int c)
{
  return fputc(c, stdout);
}

int getc()
{
  return fgetc(stdin);
}

int strlen(const char *str)
{
  int len = 0;

  if(str == (char *)0)
    return 0;

  while(*str++ != 0)
    len++;

  return len;
}

#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define LEFT    4              /* left justified */
#define LARGE   8              /* use 'ABCDEF' instead of 'abcdef' */

#define do_div(n,base) ({ \
        int __res; \
        __res = ((unsigned) n) % (unsigned) base; \
        n = ((unsigned) n) / (unsigned) base; \
        __res; \
})

int number(FILE *fp, int num, int base, int size, int precision, int type)
{
  char c, sign, tmp[66];
  const char *digits="0123456789abcdef";
  int i;

  if (type & LARGE)
    digits = "0123456789ABCDEF";
  if (type & LEFT)
    type &= ~ZEROPAD;
  if (base < 2 || base > 16)
    return 0;
  c = (type & ZEROPAD) ? '0' : ' ';
  sign = 0;

  if(type & SIGN && num < 0)
    {
      sign = '-';
      num = -num;
      size--;
    }
  
  i = 0;
  if(num == 0)
    tmp[i++] = digits[0];
  else while(num != 0)
    tmp[i++] = digits[do_div(num, base)];

  if(i > precision)
    precision = i;
  size -= precision;
  
  if(!(type&(ZEROPAD+LEFT)))
    while(size-->0)
      fputc(' ', fp);
  
  if(sign)
    fputc(sign, fp);

  if (!(type & LEFT))
    while (size-- > 0)
      fputc(c, fp);

  while (i < precision--)
    fputc('0', fp);
  
  while (i-- > 0)
    fputc(tmp[i], fp);

  while (size-- > 0)
    fputc(' ', fp);;

  return 1;
}

int vfprintf(FILE *fp, const char *fmt, va_list va)
{
  char *s;

  do
    {
      if(*fmt == '%')
        {
          bool done = false;

          int type = 0;
          int precision = 0;

          do
            {
              fmt++;
              switch(*fmt)
                {
                case '0' :
                  if(!precision)
                    type |= ZEROPAD;
                case '1' :
                case '2' :
                case '3' :
                case '4' :
                case '5' :
                case '6' :
                case '7' :
                case '8' :
                case '9' :
                  precision = precision * 10 + (*fmt - '0');
                  break;
                case '.' :
                  break;
                case 's' :
                  s = va_arg(va, char *);
                  if(!s)
                    fputs("<NULL>", fp);
                  else
                    fputs(s, fp);
                  done = true;
                  break;
                case 'c' :
                  fputc(va_arg(va, int), fp);
                  done = true;
                  break;
                case 'd' :
                  number(fp, va_arg(va, int), 10, 0, precision, type);
                  done = true;
                  break;
                case 'x' :
                  number(fp, va_arg(va, int), 16, 0, precision, type);
                  done = true;
                  break;
                case 'X' :
                  number(fp, va_arg(va, int), 16, 0, precision, type | LARGE);
                  done = true;
                  break;
                case '%' :
                  fputc(*fmt, fp);
                  done = true;
                default: 
                  fputc('%', fp);
                  fputc(*fmt, fp);
                  done = true;
                  break;
                }      
            }
          while(!done);
        }
      else
        fputc(*fmt, fp);
      
      fmt++;
    }
  while(*fmt != 0);
  
  return 0;
}

int fprintf(FILE *fp, const char *fmt, ...)
{
  va_list ap;
  int i;

  va_start(ap, fmt);
  i = fprintf(fp, fmt, ap);
  va_end(ap);  

  return i;
}

int printf(const char *fmt, ...)
{
  va_list ap;
  int i;

  va_start(ap, fmt);
  i = vfprintf(stdout, fmt, ap);
  va_end(ap);

  return i;
}
