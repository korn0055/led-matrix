#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include "application.h"

template <class t_element_type>
class CRingBuffer
{
public:
    typedef uint32_t IndexType;

    CRingBuffer(IndexType);

    void Initialize();
    t_element_type Get();
    void Put(t_element_type);
    void Put(t_element_type*, IndexType);

    /* Inquery Methods */
    bool IsEmpty() const;
    bool IsFull() const;

    /* operator overloading */
    t_element_type& operator[](const IndexType Index);

private:
    IndexType m_Capacity;
    IndexType m_ReadOffset;
    IndexType m_WriteOffset;

    t_element_type m_Buffer[10];

    IndexType NextForwardOffset(IndexType) const;
    //forward offset
    IndexType NextForwardOffset(IndexType, IndexType) const;


};

#endif
