#include "LedMatrix.h"

namespace lmx
{
    bool CLedMatrix::Initialize()
    {
        m_ActiveColIndex = NUMBER_OF_BOARDS;

        //configure outputs
        pinMode(RefreshTestPointPin, OUTPUT);
        digitalWrite(RclkPin, LOW);
        pinMode(RclkPin, OUTPUT);

        //configure SPI
        SPI.begin();
        SPI.setBitOrder(MSBFIRST);
        SPI.setClockDivider(SPI_CLOCK_DIV16);
        SPI.setDataMode(SPI_MODE0);

    }

    void CLedMatrix::Refresh()
    {
        //static uint16_t ActiveColIndex = NUMBER_OF_BOARDS;
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
          SPI.transfer((m_ActiveColIndex << 1) & 0xFF);
        }

        LatchShiftRegs();

        if(m_ActiveColIndex == 0)
          m_ActiveColIndex = COLUMNS_PER_BOARD - 1;
        else
          m_ActiveColIndex--;
    }

    //Public Methods
    void CLedMatrix::Clear()
    {
        for(int8_t i = 0; i <  (NUMBER_OF_BOARDS - 1); i++)
        {
            SPI.transfer(0);
        }
        LatchShiftRegs();
    }

    //Private Methods
    void CLedMatrix::LatchShiftRegs()
    {
        digitalWrite(RclkPin, HIGH);
        digitalWrite(RclkPin, LOW);

    }
}   //end namespace lmx
