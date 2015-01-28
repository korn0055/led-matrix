/*UART.h uses two interrupt-driven circular buffers to transmit and receive data via USART0*/

#ifndef UART_H
#define UART_H

#include "compiler.h"

#ifdef COMPILER_IAR
#include <stdlib.h>
#include <inavr.h>
#include <ioavr.h>
#define F_CPU 16000000UL
#endif

#ifdef COMPILER_WINAVR
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#endif

#define UART_BAUD 9600
#define UART_TXBUFFERSIZE 500
#define UART_RXBUFFERSIZE 500

#define UART_BRCOUNT      5
#define UART_BAUDRATES    {9600UL, 19200UL, 38400UL, 57600UL, 115200UL}

extern BYTE UART_byteTxBuffer[UART_TXBUFFERSIZE];
extern volatile BYTE *UART_pTxHead;
extern volatile BYTE *UART_pTxTail;
extern volatile bool UART_bTxOverflow;
extern volatile bool UART_bTxTailLagsHead;
extern BYTE UART_byteRxBuffer[UART_RXBUFFERSIZE];
extern BYTE *UART_pRxHead;
extern BYTE *UART_pRxTail;
extern BYTE UART_bRxOverflow;
extern BYTE UART_bRxTailLagsHead;

void UART_Init(unsigned long);
void UART_TxByte(BYTE);
void UART_TxUint(unsigned int);
void UART_FlushTxBuffer(void);
bool UART_RxByte(BYTE *);
void UART_FlushRxBuffer(void);
void UART_ISR_DataRegEmpty(void);
void UART_TxString(const char *);
void UART_TxBool(bool);
bool UART_RxString(char *,char, unsigned int);
bool UART_RxByteArray(BYTE *, unsigned int);
bool UART_CheckBaudRate(unsigned long);
void UART_TxBin(BYTE);
void UART_TxHex(BYTE *, BYTE);
bool UART_GetErrorStatus(void);
#endif
