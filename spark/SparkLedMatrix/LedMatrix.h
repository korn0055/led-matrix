#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "application.h"
#include "LedMatrixSystem.h"

//#define NUMBER_OF_BOARDS 4
//#define COLUMNS_PER_BOARD 15

namespace lmx
{
    class CLedMatrix
    {
    public:
        const int8_t NUMBER_OF_BOARDS = 5;
        const int8_t COLUMNS_PER_BOARD = 15;

        bool Initialize();
        void Refresh();
        void Clear();
    private:
        volatile uint16_t m_ActiveColIndex;

        void LatchShiftRegs();
    };
}   //end namespace lmx


#endif
