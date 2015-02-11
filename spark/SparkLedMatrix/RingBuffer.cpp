#include "RingBuffer.h"

//public methods
template<class t_element_type>
CRingBuffer<t_element_type>::CRingBuffer(IndexType Capacity)
{
    m_Capacity = Capacity;
    m_Buffer = new t_element_type[m_Capacity];
    Initialize();
}

template<class t_element_type>
CRingBuffer<t_element_type>::~CRingBuffer()
{
    delete[] m_Buffer;
}

template<class t_element_type>
void CRingBuffer<t_element_type>::Initialize()
{
    m_WriteOffset = 0;
    m_ReadOffset = 0;
}

template<class t_element_type>
t_element_type CRingBuffer<t_element_type>::Get()
{
    t_element_type Element ;
    Element = m_Buffer[m_ReadOffset];
    m_ReadOffset = NextForwardOffset(m_ReadOffset);
    return Element;
}

template<class t_element_type>
t_element_type CRingBuffer<t_element_type>::Peek()
{
    return m_Buffer[m_ReadOffset];
}

template<class t_element_type>
void CRingBuffer<t_element_type>::Put(t_element_type Element)
{
    m_Buffer[m_WriteOffset] = Element;
    m_WriteOffset = NextForwardOffset(m_WriteOffset);
}

template<class t_element_type>
void CRingBuffer<t_element_type>::Put(t_element_type * pElements, CRingBuffer<t_element_type>::IndexType Length)
{
    for(IndexType i = 0; i < Length; i++)
        Put(pElements[i]);
}

//Accessor methods
template<class t_element_type>
t_element_type& CRingBuffer<t_element_type>::operator[](const typename CRingBuffer<t_element_type>::IndexType Index)
{
    return m_Buffer[NextForwardOffset(m_ReadOffset, Index)];
}

//Inquiry methods
template<class t_element_type>
bool CRingBuffer<t_element_type>::IsEmpty() const
{
    return (m_ReadOffset == m_WriteOffset);
}

template<class t_element_type>
bool CRingBuffer<t_element_type>::IsFull() const
{
    return (m_ReadOffset == NextForwardOffset(m_WriteOffset));
}


//private methods
template<class t_element_type>
typename CRingBuffer<t_element_type>::IndexType CRingBuffer<t_element_type>::NextForwardOffset(IndexType CurrentOffset) const
{
    return (CurrentOffset == (m_Capacity - 1) ? 0 : CurrentOffset + 1);
}

//private methods
template<class t_element_type>
typename CRingBuffer<t_element_type>::IndexType CRingBuffer<t_element_type>::NextForwardOffset(IndexType CurrentOffset, IndexType Index) const
{
    if(CurrentOffset + Index > (m_Capacity - 1))
        return (Index - (m_Capacity - CurrentOffset));
    else
        return (CurrentOffset + Index);
}

//this is needed so that the compiler will include this specific implementation
template class CRingBuffer<uint8_t>;
