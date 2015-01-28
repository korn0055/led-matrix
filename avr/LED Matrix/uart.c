#include "uart.h"
/*UART.h uses two interrupt-driven circular buffers to transmit and receive data via USART0*/

BYTE UART_byteTxBuffer[UART_TXBUFFERSIZE];
volatile BYTE *UART_pTxHead;
volatile BYTE *UART_pTxTail;
volatile bool UART_bTxOverflow;
volatile bool UART_bTxTailLagsHead;
BYTE UART_byteRxBuffer[UART_RXBUFFERSIZE];
BYTE *UART_pRxHead;
BYTE *UART_pRxTail;
BYTE UART_bRxOverflow;
BYTE UART_bRxTailLagsHead;

void UART_Init(unsigned long ulBaudrate)
{
  //set baud rate
  unsigned int ubrr;
  if(ulBaudrate < 38000)
  {
   ubrr = F_CPU/16/ulBaudrate-1;
  }
  else
  {
    ubrr = F_CPU/8/ulBaudrate-1;
    UCSR0A = (1<<U2X0);
  }
  UBRR0H = (BYTE) (ubrr >> 8);
  UBRR0L = (BYTE) ubrr;
  //enable receiver, transmitter
  UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
  //set 8data, 1 stop bit
  UCSR0C = (3<<UCSZ00);

  UART_FlushTxBuffer();
  UART_FlushRxBuffer();
}

#ifdef __AVR__
ISR(USART_UDRE_vect)
#endif
#ifdef COMPILER_IAR
#pragma vector = USART0_UDRE_vect
__interrupt void USART_UDRE_vect(void)
#endif
{
  bool bufferEmpty = false;

  if ( UCSR0A & (1<<UDRE0) )
  {
    if (UART_pTxHead == UART_pTxTail)
    {
      if(UART_bTxTailLagsHead)
      {
        UART_bTxOverflow = true;
      }
      else
      {
        bufferEmpty = true;
      }
    }

    if(!bufferEmpty)
    {
      UDR0 = *UART_pTxHead;
      if (UART_pTxHead == UART_byteTxBuffer + sizeof(BYTE) * (UART_TXBUFFERSIZE - 1))
      {
        UART_pTxHead = UART_byteTxBuffer;
        UART_bTxTailLagsHead = false;
      }
      else
        UART_pTxHead++;
    }
    else
    {
      UCSR0B &= ~(1<<UDRIE0);
    }
  }
}

#ifdef __AVR__
ISR(USART_RX_vect)
#endif
#ifdef COMPILER_IAR
#pragma vector = USART0_RX_vect
__interrupt void USART_RX_vect(void)
#endif
{
  if( UCSR0A & (1<<RXC0) )
  {
    if ((UART_pRxHead == UART_pRxTail) && UART_bRxTailLagsHead)
    {
      UART_bRxOverflow = true;
    }
    else
    {
      *UART_pRxTail = UDR0;
      if (UART_pRxTail == (UART_byteRxBuffer + sizeof(BYTE) * (UART_RXBUFFERSIZE - 1)))
      {
        UART_pRxTail = UART_byteRxBuffer;
        UART_bRxTailLagsHead = true;
      }
      else
        UART_pRxTail++;
    }
  }
}

void UART_FlushTxBuffer(void)
{
  UCSR0B &= ~(1<<UDRIE0);
  UART_pTxHead = UART_pTxTail = UART_byteTxBuffer;
  UART_bTxTailLagsHead = false;
  UART_bTxOverflow = false;
}

void UART_FlushRxBuffer(void)
{
  UART_pRxHead = UART_pRxTail = UART_byteRxBuffer;
  UART_bRxTailLagsHead = false;
  UART_bRxOverflow = false;
}

void UART_TxString(const char * str)
{
  for (int i = 0; str[i] != '\0'; i++)
  {
    UART_TxByte(str[i]);
  }
}

void UART_TxBool(bool b)
{
  if(b)
  {
    UART_TxString("TRUE");
  }
  else
  {
    UART_TxString("FALSE");
  }
}

bool UART_RxString(char * str, char rDelim, unsigned int rMaxChars)
{
#ifdef COMPILER_IAR
  unsigned char savedInterrupt = __save_interrupt();
  __disable_interrupt();
#elif defined(__AVR__)
  //TODO: figure out how to save the interrupts in WINAVR
  cli();
#else
#error "no compiler defined!"
#endif

  //unsigned int uiLength = 0;
  BYTE ch;
  BYTE * pSavedRxHead;
  pSavedRxHead = UART_pRxHead;
  bool bSavedRxTailLagsHead = UART_bRxTailLagsHead;
  bool bStringAvailable = false;

  for(int i = 0; UART_RxByte(&ch); i++)
  {
    if((ch == rDelim) || i == rMaxChars)
    {
      str[i] = '\0';
      bStringAvailable = true;
      break;
    }
    else
    {
      str[i] = ch;
    }
  }

  if(!bStringAvailable)
  {
    UART_pRxHead = pSavedRxHead;
    UART_bRxTailLagsHead = bSavedRxTailLagsHead;
  }

#ifdef COMPILER_IAR
  __restore_interrupt(savedInterrupt);
#elif defined(__AVR__)
  //TODO: figure out how to save the interrupts in WINAVR
  sei();
#else
#error "no compiler defined!"
#endif

  return bStringAvailable;
}

bool UART_RxByteArray(BYTE * pBytes, unsigned int uiSize)
{
#ifdef COMPILER_IAR
  unsigned char savedInterrupt = __save_interrupt();
  __disable_interrupt();
#elif defined(__AVR__)
  cli();
#endif

  //unsigned int uiLength = 0;
  BYTE ch;
  BYTE * pSavedRxHead;
  pSavedRxHead = UART_pRxHead;
  bool bSavedRxTailLagsHead = UART_bRxTailLagsHead;
  bool bStringAvailable = false;
  bool bByteAvailable = true;

  for(int i = 0; bByteAvailable ; i++)
  {
    if(i == uiSize)
    {
      bStringAvailable = true;
      break;
    }

    bByteAvailable = UART_RxByte(&ch);
    pBytes[i] = ch;
  }

  if(!bStringAvailable)
  {
    UART_pRxHead = pSavedRxHead;
    UART_bRxTailLagsHead = bSavedRxTailLagsHead;
  }
#ifdef COMPILER_IAR
  __restore_interrupt(savedInterrupt);
#elif defined (__AVR__)
  sei();
#endif
  return bStringAvailable;

}


void UART_TxByte(unsigned char data)
{
  if (UART_pTxHead == UART_pTxTail)
  {
    if(UART_bTxTailLagsHead)
      UART_bTxOverflow = true;
//    else
//      initTx = true;
  }
  if(!UART_bTxOverflow)
  {
    *UART_pTxTail = data;
    if (UART_pTxTail == (UART_byteTxBuffer + sizeof(BYTE) * (UART_TXBUFFERSIZE - 1)))
    {
      UART_pTxTail = UART_byteTxBuffer;
      UART_bTxTailLagsHead = true;
    }
    else
      UART_pTxTail++;
  }

  UCSR0B |= (1<<UDRIE0);
//  if(initTx)
//    UDR0 = 'x';

}

bool UART_RxByte(BYTE *data)
{
  if (UART_pRxHead == UART_pRxTail)
  {
    if(UART_bRxTailLagsHead)
    {
      UART_bRxOverflow = true;
    }
    return false;
  }
  else
  {
    *data = *UART_pRxHead;
    if (UART_pRxHead == UART_byteRxBuffer + sizeof(BYTE) * (UART_RXBUFFERSIZE - 1))
    {
      UART_pRxHead = UART_byteRxBuffer;
      UART_bRxTailLagsHead = false;
    }
    else
    {
      UART_pRxHead++;
    }
  }
  return true;
}

void UART_TxUint(unsigned int data)
{
  unsigned char quotient = 0;

  for (unsigned int divisor = 10000; divisor > 0; divisor /= 10)
  {
    quotient = data / divisor;
    UART_TxByte(quotient + 0x30);
    data -= quotient * divisor;
  }
}

bool UART_CheckBaudRate(unsigned long ulBaudRate)
{
  unsigned long baudrates[UART_BRCOUNT] = UART_BAUDRATES;

  for(int i = 0; i < UART_BRCOUNT; i++)
  {
    if (ulBaudRate == baudrates[i])
    {
      return true;
    }
  }
  return false;
}

void UART_TxBin(BYTE data)
{
  for (int i = 7; i >= 0; i--)
  {
    if(data & (1 << i))
    {
      UART_TxByte('1');
    }
    else
    {
      UART_TxByte('0');
    }
  }
}

void UART_TxHex(BYTE * data, BYTE bytLength)
{
  for (int i = bytLength - 1; i >= 0; i--)
  {
    for (int j = 4; j >=0; j -= 4)
    {
      BYTE nibble = (data[i] >> j) & 0x0f;
      if (nibble > 9)
      {
        UART_TxByte(nibble + 0x41 - 10);
      }
      else
      {
        UART_TxByte(nibble + 0x30);
      }
    }
  }
}

bool UART_GetErrorStatus(void)
{
	return (UART_bTxOverflow || UART_bRxOverflow);
}






