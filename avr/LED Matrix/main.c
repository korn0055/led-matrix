/*
 * main.c
 *
 *  Created on: Feb 16, 2011
 *      Author: Ryan
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "font.h"

#define NUMBER_MATRIX_COLUMNS	15
#define NUMBER_MATRIX_ROW		7


enum DISP_JUSTIFICATION
{
	LEFT_JUSTIFIED = 0,
	RIGHT_JUSTIFIED,
	CENTER_JUSTIFIED
};


void RefreshDisplay(unsigned char);
void ShiftMatixVertically(char *, const char *, const char *, int);
unsigned int RenderDotMatrix(char * renderedMatrix, const char *ascii, int offset, enum DISP_JUSTIFICATION);

volatile static unsigned long x = 1;
volatile static unsigned long millisecond_ticks = 0;
static char displayBuffer[NUMBER_MATRIX_COLUMNS];

int main(void)
{
	int offset = (-NUMBER_MATRIX_COLUMNS);
	//int offset = 0;
	int msgSize = 0;

	DDRB = (1<<PB5)|(1<<PB3)|(1<<PB2)|(1<<PB1);
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPOL)|(1<<CPHA)|(0<<SPR0)|(0<<SPR0);
	PORTB = 0;
	DDRC = (1<<PC5);
	DDRD = (1<<PD6);

	//1ms display timer
	TCCR0A = (1<<COM0A0) | (1 << WGM01) | (0 << WGM00); 		//CTC mode
	TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (1 << CS00);
	OCR0A = 250;
	TIMSK0 = (1 << OCIE0A);

	sei();

	while(1)
	{
		char displayData[NUMBER_MATRIX_COLUMNS];
		//char bottomData[NUMBER_MATRIX_COLUMNS];
		//char allOn[NUMBER_MATRIX_COLUMNS] = {0x7f};
		//char allOff[NUMBER_MATRIX_COLUMNS] = {0x00};

		msgSize = RenderDotMatrix(displayData, "...And furthermore Susan, I wouldn't be the least bit surprised to learn that all four of them habitually smoked marijuana cigarettes. REEFERS!!" , offset, LEFT_JUSTIFIED);
		msgSize = RenderDotMatrix(displayData, "Does this stay bright @ .01 sec refresh rate??", offset, LEFT_JUSTIFIED);
		//RenderDotMatrix(bottomData, " Down",0);
		//ShiftMatixVertically(displayData, displayData, bottomData, offset*(-1));
		//char longString[6];
		//RenderDotMatrix(displayData, ltoa(offset, longString, 10),0);
		cli();
		for(unsigned int i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
			displayBuffer[i] = displayData[i];

		if(millisecond_ticks > 150)
		{
			offset++;
			millisecond_ticks = 0;
		}
		if(offset > msgSize + NUMBER_MATRIX_COLUMNS)
		{
			offset = (-NUMBER_MATRIX_COLUMNS);
			//offset=0;
		}

		sei();

	}

	return 0;
}

ISR(TIMER0_COMPA_vect)
{
	static unsigned char activeCol = 0;
	if( (millisecond_ticks % 1) == 0 )
	{
		RefreshDisplay(activeCol++);
		if (activeCol == NUMBER_MATRIX_COLUMNS)
			activeCol = 0;
	}
	millisecond_ticks++;
}


void RefreshDisplay(unsigned char col)
{
	SPDR = (1 << col) >> 8;
	while( !(SPSR & (1<<SPIF)) )
			;
	SPDR = 1 << col;
	while( !(SPSR & (1<<SPIF)) )
			;
	SPDR = displayBuffer[col];
	while( !(SPSR & (1<<SPIF)) )
			;

	PORTB |= (1<<PB2);
	PORTB &= ~(1<<PB2);
	/*  toggle test point
	if(PORTC & (1<<PC5))
		PORTC &= ~(1<<PC5);
	else
		PORTC |= (1<<PC5);
		*/
}

unsigned int RenderDotMatrix(char * renderedMatrix, const char *ascii, int offset, enum DISP_JUSTIFICATION just)// DISP_JUSTIFICATION just)
{
	unsigned int columnIndex = 0;
	unsigned int renderedIndex = 0;
	char temp[NUMBER_MATRIX_COLUMNS];

	for (unsigned char i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
	{
		renderedMatrix[i] = 0;
	}

	if (offset < 0)
	{
		renderedIndex = offset * (-1);
		offset = 0;
	}

	while(*ascii != '\0')
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

	if(just)
	{
		for (unsigned int i = 0; i < NUMBER_MATRIX_COLUMNS; i++)
		{
			temp[i] = renderedMatrix[i];
			renderedMatrix[i] = 0;
		}
		for (unsigned int i = 0; i < renderedIndex; i++)
		{
			renderedMatrix[i + ((NUMBER_MATRIX_COLUMNS + 1) - renderedIndex)/just] = temp[i];
		}
	}
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
