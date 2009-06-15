//*----------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : main.h
//* Object              :
//*
//* 1.0 27/03/03 HIi    : Creation
//* 1.01 03/05/04 HIi   : AT9C_VERSION incremented to 1.01
//*----------------------------------------------------------------------------

#ifndef main_h
#define main_h

#include "embedded_services.h"

// crystal= 18.432MHz
// MULA = 38, DIVA = 4, (MULA + 1) / DIV = 9.75
#define AT91C_PLLA_VALUE 				  0x2026BE04 // -> 179.712MHz
#define AT91C_PLLB_VALUE 				  0x10483E0E

#define DELAY_MAIN_FREQ				  1000

#define AT91C_MASTER_CLOCK              60000000
#define AT91C_BAUD_RATE                 115200

#define AT91C_SDRAM_BASE_ADDRESS        0x20000000
#define AT91C_SDRAM_SIZE                0x04000000

#define AT91C_OFFSET_VECT6              0x14        //* Offset for ARM vector 6

#define AT91C_NAME		  "LAB memtest"
#define AT91C_VERSION   "1.0"

// Global variables and functions definition
extern unsigned int GetTickCount(void);
#endif
