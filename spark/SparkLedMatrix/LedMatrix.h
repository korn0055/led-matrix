#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "application.h"
#include "LedMatrixSystem.h"
#include "RingBuffer.h"

//#define NUMBER_OF_BOARDS 4
//#define COLUMNS_PER_BOARD 15

namespace lmx
{
    class CLedMatrix
    {
    public:
        const int8_t NUMBER_OF_BOARDS = 4;
        const int8_t COLUMNS_PER_BOARD = 15;
        const uint32_t PIXEL_BUFFER_SIZE = NUMBER_OF_BOARDS * COLUMNS_PER_BOARD + 1;
        const uint32_t TEXT_BUFFER_SIZE = 100;

        CLedMatrix();

        bool Initialize();
        void Refresh();
        void Clear();
    private:
        volatile uint16_t m_ActiveColIndex;
        CRingBuffer<uint8_t> m_PixelBuffer;
        CRingBuffer<uint8_t> m_TextBuffer;

        void LatchShiftRegs();
    };
}   //end namespace lmx


#endif
