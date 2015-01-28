/*
 * LED_Matrix.c
 *
 * Created: 7/28/2012 8:34:54 PM
 *  Author: Ryan
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "font.h"
#include "uart.h"
#include "ring_buffer.h"

#define NUMBER_BOARDS			4
#define COLUMNS_PER_BOARD		15
#define NUMBER_MATRIX_COLUMNS	60
#define LAST_COLUMN_INDEX		(NUMBER_MATRIX_COLUMNS-1)
#define NUMBER_MATRIX_ROW		7
#define SCANS_PER_SCROLL		15
const unsigned int COLS_SCANNED_PER_SCROLL = SCANS_PER_SCROLL * COLUMNS_PER_BOARD;

typedef struct disp_state_tag disp_state_t;

struct disp_state_tag
{
	//uint8_t *pPixelBufferHead;
	//uint8_t *pPixelBufferTail;
	//uint8_t *pActiveColData[NUMBER_BOARDS];
	//uint8_t *pStringBufferHead;
	//uint8_t *pStringTargetChar;
	ring_buffer_t *pPixelRingBuffer;
	ring_buffer_t *pStringRingBuffer;	
	uint8_t uiCharIndex;
	uint8_t uiSubCharIndex;
	uint8_t uiLeadingBlanks;
	uint8_t uiTrailingBlanks;
	//uint8_t uiActiveColIndex;
	uint8_t uiTimeIndex;
};


enum DISP_JUSTIFICATION
{
	LEFT_JUSTIFIED = 0,
	RIGHT_JUSTIFIED,
	CENTER_JUSTIFIED
};

//unsigned int numCols = 10;

void RefreshDisplay(unsigned char, unsigned int);
//void ShiftMatixVertically(char *, const char *, const char *, int);
//volatile int RenderDotMatrix(volatile char * renderedMatrix, const char *ascii, int offset, enum DISP_JUSTIFICATION, bool bInvert);
//void DisplayAndScroll(void);
//unsigned int TestRender(char * renderedMatrix, const char *ascii, volatile int offset, enum DISP_JUSTIFICATION just, bool bInvert);
//void TranslateAndRender(volatile char * displayBuffer, char * const ascii);
void LoadActiveRow(disp_state_t *, uint16_t, bool);
void Render(disp_state_t * pState);
void ClearDisplay(void);

#define STRING_BUFFER_SIZE 200	//must be < 255

static uint8_t displayBuffer[NUMBER_MATRIX_COLUMNS+1] = {0};
static uint8_t startupString[] = "Startup text here...";//"A few years ago, [Paul]'s son got a simple electronic toy that plays funny noises and sings to him.";// The son loves the toy, but after months and months of use, the toy was inevitably broken beyond repair. Figuring an ‘electronic box that plays sounds’ wouldn’t be a hard project to replicate";
static uint8_t displayString[STRING_BUFFER_SIZE];// = "ABCDEFGH123";
static uint8_t whitespace_count = 0;

volatile disp_state_t gState;
volatile ring_buffer_t gPixelRingBuffer;

int main(void)
{		
	DDRB = (1<<PB5)|(1<<PB3)|(1<<PB2)|(1<<PB1);
	/*
	//SPIE = 0, no interrupt
	//SPE = 1, SPI enabled
	//MSTR = 1, master mode  (must set /SS as output)
	//CPOL = 0, Leading Edge = Rising, Trailing Edge = Falling
	//CPHA = 0, Leading Edge = Sample, Trailing Edge = Setup
	//SPI2X (in SPSR) :SPR1:SPR2 = 001 = fosc/16 = 1MHz	*/
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);	
	SPSR = (1<<SPI2X);
	PORTB = 0;
	DDRC = ((1<<PC5) | (1<<PC4) | (1 <<PC3));
	/*
	//PD0 - RXD
	//PD1 - TXD
	//PD6 - OC0A*/
	DDRD = (1<<PD6) | (1<<PD1);
	
	/*
	//1ms display timer
	TCCR0A = (0<<COM0A1) |(1<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1 << WGM01) | (0 << WGM00); 		//CTC mode
	TCCR0B = (0 << WGM02) | (1 << CS02) | (0<< CS01) | (0 << CS00);	//100 = clock prescaler = 256 ~4ms timer
	OCR0A = 250;
	TIMSK0 = (1 << OCIE0A);
	*/
	
	
	//100us display timer
	TCCR0A = (0 << COM0A1) |(1<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (1 << WGM01) | (0 << WGM00); 		//CTC mode
	TCCR0B = (0 << WGM02) | (0 << CS02) | (1<< CS01) | (0 << CS00);	//100 = clock prescaler = 256 ~4ms timer
	OCR0A = 200;
	TIMSK0 = (1 << OCIE0A);
	
	
	
	gPixelRingBuffer = ring_buffer_init(displayBuffer, NUMBER_MATRIX_COLUMNS+1);
	ring_buffer_t gStringRingBuffer = ring_buffer_init(displayString, 200);
		
	gState.pPixelRingBuffer = &gPixelRingBuffer;
	gState.pStringRingBuffer = &gStringRingBuffer;
	gState.uiTimeIndex = 0;	
	gState.uiSubCharIndex = 0;
	gState.uiLeadingBlanks = 0;
	gState.uiTrailingBlanks = NUMBER_MATRIX_COLUMNS;

	
	for(uint8_t i = 0; i < sizeof(startupString); i++)
	{
		ring_buffer_put(gState.pStringRingBuffer, startupString[i]);
	}
	
	sei();
	
	UART_Init(9600);		
	while(1)	
	{		
					
		char rxString[STRING_BUFFER_SIZE];		
		
		if(UART_RxString(rxString, '\r',STRING_BUFFER_SIZE))
		{
			while(!ring_buffer_is_empty(&gStringRingBuffer))
				;
			cli();	//block interrupts so that displayString isn't overwritten if a scan occurs						
			gStringRingBuffer = ring_buffer_init(displayString, STRING_BUFFER_SIZE);
			for(unsigned int i=0; rxString[i] != '\0' && !ring_buffer_is_full(&gStringRingBuffer); i++)
			{				
				ring_buffer_put(&gStringRingBuffer, rxString[i]);
			}			
			ring_buffer_put(&gStringRingBuffer, '\0');			
			sei();
			UART_TxByte('\r');
			UART_TxString(rxString);			
		}
		
		
		//if(ring_buffer_is_empty(&gStringRingBuffer))
		//{
			//cli();
			//if(rxValid)
			//{
				//for(unsigned int i=0; rxString[i] != '\0' && !ring_buffer_is_full(&gStringRingBuffer); i++)
				//{
					//ring_buffer_put(&gStringRingBuffer, rxString[i]);
				//}				
			//}
			//else
			//{
				//for(uint8_t i = 0; i < sizeof(rxString); i++)
				//{
					//ring_buffer_put(gState.pStringRingBuffer, startupString[i]);
				//}
			//}
			//gState.uiTrailingBlanks = NUMBER_MATRIX_COLUMNS;
			//sei();
		//}		
		
		
		
		if(UART_GetErrorStatus())
		{
			UART_FlushTxBuffer();
			UART_FlushRxBuffer();
			UART_TxString("UART buffer overflow!");
		}	
			
	}

	return 0;
}

/*
ISR(TIMER0_COMPA_vect)
{
	static unsigned char activeCol = COLUMNS_PER_BOARD - 1;
	static unsigned char scanCount = 0;
	static bool scanDirection = true;
	static unsigned char startCol = (COLUMNS_PER_BOARD - 1);
	static unsigned char colsScannedThisScan = 0;	
	static unsigned char colsScannedThisScroll = 0;
	static unsigned char scrollCount = 0;

	LoadActiveRow(&gState);
	//RefreshDisplay(activeCol, colsScannedThisScroll );										
	colsScannedThisScan++;	
	colsScannedThisScroll++;
	if (colsScannedThisScan == COLUMNS_PER_BOARD)
	{
		colsScannedThisScan = 0;
		if(startCol == 0)
		{
			startCol = (COLUMNS_PER_BOARD - 1);						
		}
		else
		{
			startCol--;			
		}

		activeCol =  startCol;
		if (scanCount == SCANS_PER_SCROLL)
		{	
			scanCount = 0;
			colsScannedThisScroll = 0;
			PORTC |= (1<<PC4);
			if(scrollCount % 1 == 0)
			{
				//TranslateAndRender(displayBuffer, displayString);
				Render(&gState);
			}
			scrollCount++;			
			PORTC &= ~(1<<PC4);			
		}
		scanCount++;		
	}
	else
	{				
		if (activeCol == 0)
		{
			activeCol = COLUMNS_PER_BOARD - 1;
		}
		else
		{
			activeCol--;
		}
	}
			
					
	//if (activeCol == COLUMNS_PER_BOARD && scanDirection == true)
	//{
		//scanDirection = false;
		//PORTC |= (1<<PC4);
		//DisplayAndScroll();
		//PORTC &= ~(1<<PC4);
	//}
	//else if(activeCol == 0 && scanDirection == false)		
	//{		
		//scanDirection = true;
		////PORTC |= (1<<PC4);
		////DisplayAndScroll();
		////PORTC &= ~(1<<PC4);
	//}
	//else
	//{
		//if(scanDirection == true)
			//activeCol++;
		//else
			//activeCol--;		
	//}
}
*/

ISR(TIMER0_COMPA_vect)
{
	static unsigned char activeCol = COLUMNS_PER_BOARD - 1;
	static unsigned char scanCount = 0;		
	static unsigned char colsScannedThisScan = 0;
	static unsigned char colsScannedThisScroll = 0;
	
	if (scanCount == SCANS_PER_SCROLL / 3)
	{
		ClearDisplay();
	}
	else if (scanCount < ((SCANS_PER_SCROLL * 1) /  3))
	{
		LoadActiveRow(&gState, activeCol, false);
	}
	else if (scanCount > ((SCANS_PER_SCROLL * 2) /  3))
	{
		LoadActiveRow(&gState, activeCol, true);
	}
			
	colsScannedThisScan++;
	colsScannedThisScroll++;
	if (colsScannedThisScan == COLUMNS_PER_BOARD)
	{
		colsScannedThisScan = 0;
		
		if (scanCount == SCANS_PER_SCROLL)
		{
			scanCount = 0;
			colsScannedThisScroll = 0;
			PORTC |= (1<<PC4);
			if(!ring_buffer_is_empty(&gPixelRingBuffer))
			{
				ring_buffer_pop(&gPixelRingBuffer);	//scroll	
			}						
			Render(&gState);
												
			PORTC &= ~(1<<PC4);
		}
		scanCount++;
	}
	else
	{
		if (activeCol == 0)
		{
			activeCol = COLUMNS_PER_BOARD - 1;				
		}
		else
		{
			activeCol--;			
		}		
	}
}

void LoadActiveRow(disp_state_t * pState, uint16_t active_col, bool shift_ahead)
{
	uint8_t pixel_col_data = 0;
	//set test point high
	PORTC |= (1<<PC5);	
	
	for(int board = NUMBER_BOARDS - 1; board >= 0; board--)
	{		
		SPDR = (1 << active_col) >> 8;
		while( !(SPSR & (1<<SPIF)) )
		;		
		SPDR = 1 << active_col;
				
		if(shift_ahead)
		{			
			if(board == (NUMBER_BOARDS - 1) && (active_col == (COLUMNS_PER_BOARD - 1)))
			{
				pixel_col_data = 0;
				
			}
			else
			{
				pixel_col_data = ring_buffer_index(&gPixelRingBuffer, active_col + COLUMNS_PER_BOARD * board + 1);				
			}
			
		}
		else 
		{			
			pixel_col_data = ring_buffer_index(&gPixelRingBuffer, active_col + COLUMNS_PER_BOARD * board);			
		}
		
		while( !(SPSR & (1<<SPIF)) )
		;
		SPDR = pixel_col_data; 
		while( !(SPSR & (1<<SPIF)) )
		;
	}
	
	//toggle RCLK
	PORTB |= (1<<PB2);
	PORTB &= ~(1<<PB2);
	
	////clear test point
	PORTC &= ~(1<<PC5);
}

void ClearDisplay(void)
{
	for(uint8_t bytes = 0; bytes < NUMBER_BOARDS * 3; bytes++)	
	{
		SPDR = 0; 
		while( !(SPSR & (1<<SPIF)) )
			;
	}
	
	//toggle RCLK
	PORTB |= (1<<PB2);
	PORTB &= ~(1<<PB2);
	
}

void Render(disp_state_t * pState)
{
	
	//try fast types
	uint8_t target_char;
	static uint8_t rendered_col = 0;
	
	while(!ring_buffer_is_full(pState->pPixelRingBuffer))// && !ring_buffer_is_empty(pState->pStringRingBuffer))
	{
		if(ring_buffer_is_empty(pState->pStringRingBuffer))
		{
			target_char = '\0';
		}
		else
		{
			target_char = ring_buffer_peek(pState->pStringRingBuffer);
		}
		
		rendered_col = 0;
		
		if(target_char == '\0')
		{
			if(pState->uiTrailingBlanks > 0)
			{
				pState->uiTrailingBlanks--;		//add blank column
			}
			else if(!ring_buffer_is_empty(pState->pStringRingBuffer))
			{
				ring_buffer_get(pState->pStringRingBuffer);
			}
		}
		else if(target_char >= ASCII_START && target_char <= ASCII_END)
		{			
			//move this subtraction/check out of here for speed
			rendered_col = fontArray[target_char - ASCII_OFFSET][pState->uiSubCharIndex];
			if(rendered_col == 0xff || pState->uiSubCharIndex > 4)
			{		
				rendered_col = 0;				//end of char, add blank column		
				pState->uiSubCharIndex = 0;		//advance to next ascii char
				//pState->uiCharIndex++;			//modify this if string buf is circular
				ring_buffer_get(pState->pStringRingBuffer);
			}
			else
			{
				pState->uiSubCharIndex++;
			}
		}
		else
		{
			rendered_col = 0xaa;			//invalid char
			pState->uiSubCharIndex = 0;		//advance to next aschii char
			//pState->uiCharIndex++;
			//ring_buffer_get(pState->pStringRingBuffer);
		}
				
		ring_buffer_put(pState->pPixelRingBuffer, rendered_col);
	}

}

//void Render(disp_state_t * pState)
//{		
	//while(pState->pPixelBufferTail != get_next_pixel(pState->pPixelBufferHead))
	//{
		//if(*pState->pStringTargetChar == '\0')
		//{
			//if(pState->uiTrailingBlanks > 0)
			//{
				//pState->uiTrailingBlanks--;
				//*pState->pPixelBufferTail = 0;	//add blank column				
			//}
		//}
		//else if(*pState->pStringTargetChar >= ASCII_OFFSET && *pState->pStringTargetChar < ASCII_OFFSET + FONT_ARRAY_SIZE)
		//{
			//*pState->pPixelBufferTail = fontArray[*pState->pStringTargetChar - ASCII_OFFSET][pState->uiSubCharIndex];
			//if(*pState->pPixelBufferTail == 0xff || pState->uiSubCharIndex > 4)
			//{
				//*pState->pPixelBufferTail = 0;	//add blank column				
				//pState->uiSubCharIndex = 0;		//advance to next ascii char
				//pState->pStringTargetChar++;	//modify this if string buf is circular
			//}
			//else
			//{
				//pState->uiSubCharIndex++;
			//}
		//}
		//else
		//{
			//*pState->pPixelBufferTail = 0xaa;	//add blank column
			//pState->uiSubCharIndex = 0;
			//pState->pStringTargetChar++;
		//}
		//
		//pState->pPixelBufferTail = get_next_pixel(pState->pPixelBufferHead);
	//}	
		//
//}

void TranslateAndRender(volatile char * displayBuffer,  char * const ascii )
{
	static char * asciiIndexPointer = NULL;
	static unsigned asciiSubIndex = 0;
	static uint8_t trailingBlankScrollCount = 0;	
			
	if(asciiIndexPointer == NULL)
	{
		asciiIndexPointer = ascii;
	}			
	
	if(*asciiIndexPointer == '\0')
	{
		if(trailingBlankScrollCount > NUMBER_MATRIX_COLUMNS)
		{
			asciiIndexPointer = ascii;
			asciiSubIndex = 0;			//this should be redundant
			trailingBlankScrollCount = 0;
		}
		else
		{
			trailingBlankScrollCount++;
		}			
	}
	
	
	for (unsigned char bufferIndex = 0; bufferIndex < NUMBER_MATRIX_COLUMNS; bufferIndex++)
	{
		char bufferRowData = 0;

		if(bufferIndex < NUMBER_MATRIX_COLUMNS - 1)
		{
			//shift buffer array by one
			bufferRowData = displayBuffer[bufferIndex + 1];
		}
		else
		{		
			if(*asciiIndexPointer != '\0')
			{
				if(*asciiIndexPointer >= ASCII_OFFSET && *asciiIndexPointer < ASCII_OFFSET + FONT_ARRAY_SIZE)
				{
					bufferRowData = fontArray[*asciiIndexPointer - ASCII_OFFSET][asciiSubIndex];
					if(bufferRowData == 0xff || asciiSubIndex > 4)
					{
						bufferRowData = 0;	//add blank column
						asciiSubIndex = 0;	//advance to next ascii char
						asciiIndexPointer++;					
					}
					else
					{
						asciiSubIndex++;	
					}		
				}
				else //undefined character
				{
						bufferRowData = 0xaa;	//add bogus column
						asciiSubIndex = 0;	//advance to next ascii char
						asciiIndexPointer++;	
				}				

			}
																		
		}
		displayBuffer[bufferIndex] = bufferRowData;
	}
}

/*
unsigned int TestRender(char * renderedMatrix, const char *ascii, int offset, enum DISP_JUSTIFICATION just, bool bInvert)// DISP_JUSTIFICATION just)
{
	for (unsigned char i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
	{
		//if(i >= offset && (i < (offset + 5)))
		if(i==offset)
		{			
			//renderedMatrix[i] = (1<<(offset/5));
			renderedMatrix[i] = 0x7f;
		}
		else
		{
			renderedMatrix[i] = 0;
		}
		
	}
	return -1;
	
};

volatile int RenderDotMatrix(volatile char * renderedMatrix, const char *ascii, volatile int offset, enum DISP_JUSTIFICATION just, bool bInvert)// DISP_JUSTIFICATION just)
{
	int columnIndex = 0;
	unsigned int renderedIndex = 0;
	char temp[NUMBER_MATRIX_COLUMNS];

	//clear output array
	for (unsigned char i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
	{
		renderedMatrix[i] = 0;
	}
	
	if (offset < 0)
	{
		renderedIndex = offset * (-1);
		offset = 0;
	}
	
	while(*ascii != '\0' && renderedIndex < NUMBER_MATRIX_COLUMNS)
	{
		for(unsigned char charCol = 0; charCol < 5; charCol++)
		{
			unsigned char rowData = fontArray[*ascii - ASCII_OFFSET][charCol];
			if( (columnIndex >= offset) && (rowData != 0xff) && (renderedIndex < NUMBER_MATRIX_COLUMNS) )
			{
				renderedMatrix[renderedIndex] = rowData;
				renderedIndex++;
	
			}
			if(rowData != 0xff)
				columnIndex++;		
		}
		//add blank line between characters
		if(renderedIndex < NUMBER_MATRIX_COLUMNS)
		{
			if(columnIndex >= offset)
			{
				renderedIndex++;
			}
			columnIndex++;
		}
		ascii++;
	}

	//if(just)
	//{
		//for (unsigned int i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
		//{
			//temp[i] = renderedMatrix[i];
			//renderedMatrix[i] = 0;
		//}
		//for (unsigned int i = 0; i < renderedIndex; i++)
		//{
			//renderedMatrix[i + ((NUMBER_MATRIX_COLUMNS + 1) - renderedIndex)/just] = temp[i];
		//}
	//}
	//
	//if(bInvert)
	//for (unsigned int i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
	//{		
		//renderedMatrix[i] = ~renderedMatrix[i];
	//}
	//
	return columnIndex;
}

void ShiftMatixVertically(char * outputMatrix, const char * startMatrix, const char * finishMatrix, int shiftAmount)
{
	for(int i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
	{
		unsigned int stackedMatrix;
		if (shiftAmount > 0)
		{
			stackedMatrix = startMatrix[i] | (finishMatrix[i] << 8);
			stackedMatrix >>= shiftAmount;
			outputMatrix[i] = (stackedMatrix & 0xff);
		}
		else
		{
			stackedMatrix = (startMatrix[i] << 8) | finishMatrix[i];
			stackedMatrix <<= shiftAmount * (-1);
			outputMatrix[i] = (stackedMatrix >> 8);
		}
	}
}
*/