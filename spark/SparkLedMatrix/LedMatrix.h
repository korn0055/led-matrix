#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "application.h"
#include "LedMatrixSystem.h"
#include "RingBuffer.h"

namespace lmx
{
    class CLedMatrix
    {
    public:
        const uint8_t NUMBER_OF_BOARDS = 4;
        const uint8_t COLUMNS_PER_BOARD = 15;
        const uint8_t NUMBER_OF_COLUMNS = NUMBER_OF_BOARDS * COLUMNS_PER_BOARD;
        const uint32_t PIXEL_BUFFER_SIZE = NUMBER_OF_COLUMNS + 1;
        const uint32_t TEXT_BUFFER_SIZE = 30;

        CLedMatrix();

        uint16_t PutText(char [], uint16_t);
        bool Initialize();
        void Refresh();
        void Clear();
    private:
        volatile uint16_t m_ActiveColIndex;
        volatile uint16_t m_TrailingBlanks;
        volatile uint8_t m_SubCharIndex;

        CRingBuffer<uint8_t> m_PixelBuffer;
        CRingBuffer<uint8_t> m_TextBuffer;

        void LatchShiftRegs();
        void Render();
    };
}   //end namespace lmx


#endif
