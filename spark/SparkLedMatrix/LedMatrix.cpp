#include "LedMatrix.h"
#include "Font.h"

namespace lmx
{
    CLedMatrix::CLedMatrix() :
        m_PixelBuffer(PIXEL_BUFFER_SIZE),
        m_TextBuffer(TEXT_BUFFER_SIZE)
    {

    }

    bool CLedMatrix::Initialize()
    {
        m_ActiveColIndex = NUMBER_OF_BOARDS;
        m_TrailingBlanks = NUMBER_OF_COLUMNS;
        m_SubCharIndex = 0;

        //configure outputs
        pinMode(RefreshTestPointPin, OUTPUT);
        digitalWrite(RclkPin, LOW);
        pinMode(RclkPin, OUTPUT);

        //configure SPI
        SPI.begin();
        SPI.setBitOrder(MSBFIRST);
        SPI.setClockDivider(SPI_CLOCK_DIV16);
        SPI.setDataMode(SPI_MODE0);

        m_PixelBuffer.Initialize();
        m_TextBuffer.Initialize();

        return true;
    }

    //uint16_t CLedMatrix::PutText(char Text[], uint16_t MaxChars = 0xffff)
    uint16_t CLedMatrix::PutText(String Text)
    {
        uint16_t PushCount = 0;
        /*
        while (PushCount < MaxChars && Text[PushCount] != '\0' && !m_TextBuffer.IsFull())
            m_TextBuffer.Put(Text[PushCount++]);
        */
        for(; PushCount <= Text.length() && !m_TextBuffer.IsFull(); PushCount++)
            m_TextBuffer.Put(Text.charAt(PushCount));
        //Render();
        return PushCount;
    }

    void CLedMatrix::Refresh()
    {
        static int ledState = LOW;

        if (ledState == LOW) {
          ledState = HIGH;
          PIN_MAP[RefreshTestPointPin].gpio_peripheral->BSRR = PIN_MAP[RefreshTestPointPin].gpio_pin; // LED High
        }
        else {
          ledState = LOW;
          PIN_MAP[RefreshTestPointPin].gpio_peripheral->BRR = PIN_MAP[RefreshTestPointPin].gpio_pin; // LED low
        }

        for(int8_t i = (NUMBER_OF_BOARDS - 1); i >= 0; i--)
        {
            SPI.transfer((1 << m_ActiveColIndex) >> 8);
            SPI.transfer((1 << m_ActiveColIndex) & 0xFF);
            SPI.transfer(m_PixelBuffer[m_ActiveColIndex + COLUMNS_PER_BOARD * i]);
        }

        LatchShiftRegs();

        if(m_ActiveColIndex == 0)
          m_ActiveColIndex = COLUMNS_PER_BOARD - 1;
        else
          m_ActiveColIndex--;
    }

    //Public Methods
    void CLedMatrix::BackgroundProc()
    {
        Render();
    }

    void CLedMatrix::Scroll(int16_t Columns)
    {
        m_PixelBuffer.Flush(Columns);
    }

    void CLedMatrix::FlushBuffers()
    {
        noInterrupts();
        m_TextBuffer.Flush();
        m_PixelBuffer.Flush();
        m_ActiveColIndex = NUMBER_OF_BOARDS;
        m_TrailingBlanks = NUMBER_OF_COLUMNS;
        m_SubCharIndex = 0;
        interrupts();
    }

    //Private Methods
    void CLedMatrix::ClearShiftRegs()
    {
        for(int8_t i = 0; i <  (NUMBER_OF_BOARDS - 1); i++)
        {
            SPI.transfer(0);
        }
        LatchShiftRegs();
    }

    void CLedMatrix::LatchShiftRegs()
    {
        digitalWrite(RclkPin, HIGH);
        digitalWrite(RclkPin, LOW);
    }

    void CLedMatrix::Render()
    {
        uint8_t TargetChar;
        uint8_t RenderedColumn = 0;

        while(!m_PixelBuffer.IsFull())
        {
            if(m_TextBuffer.IsEmpty())
                TargetChar = '\0';
            else
                TargetChar = m_TextBuffer.Peek();

            if(TargetChar == '\0')
            {
                if(m_TrailingBlanks > 0)
                    m_TrailingBlanks--; //add blank column
                else if(!m_TextBuffer.IsEmpty())
                    m_TextBuffer.Get(); //advance
            }
            else if(TargetChar >= ASCII_START && TargetChar <= ASCII_END)
            {
                RenderedColumn = FontArray[TargetChar - ASCII_OFFSET][m_SubCharIndex];
                if(RenderedColumn == FONT_COL_STOP_CHAR || m_SubCharIndex >= FONT_CHAR_MAX_WIDTH)
                {
                    RenderedColumn = 0; //end of char, add blank column for spacing, make this configurable later?
                    m_SubCharIndex = 0; //advance to next char
                    if(!m_TextBuffer.IsEmpty())
                        m_TextBuffer.Get(); //advance to next char
                }
                else
                    m_SubCharIndex++;
            }
            else //the character isn't defined by the font
            {
                RenderedColumn = 0xaa;  //show some invalid column
                m_SubCharIndex = 0;     //advance to next char
            }

            m_PixelBuffer.Put(RenderedColumn);

        }

    }
}   //end namespace lmx
