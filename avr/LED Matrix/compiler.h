//typedef.h
#ifndef COMPILER_H
#define COMPILER_H

typedef unsigned char BYTE;

#ifdef __AVR__
//typedef unsigned char bool;
#define true  1
#define false 0
#endif

#ifdef COMPILER_IAR
#include <stdbool.h>
#include <stdlib.h>
#include <inavr.h>
#include <ioavr.h>
#endif


#endif
