#include <stdarg.h>

struct FILE
{ 
  bool active;
  int (*put)(int);    /* function to write one char to device */
  int (*get)();       /* function to read one char from device */
};

#define FILEDESCS 8

FILE *fopen(int (*put)(int), int (*get)());
int fclose(FILE *fp);

int puts(const char *str);
int putc(int c);
int putchar(int c);
int getc();
 
int fputs(const char *str, FILE *fp);
int fputc(int c, FILE *fp);
int fgetc(FILE *fp);
int strlen(const char *str);

int fprintf(FILE *fp, const char *fmt, ...);
int vfprintf(FILE *fp, const char *fmt, va_list ap);

int printf(const char *fmt, ...);

extern FILE *stdout;
extern FILE *stdin;
