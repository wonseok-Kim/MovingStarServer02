#include "RingBuffer.h"

#include <memory.h>
#include <stdlib.h>

void RingBuffer::Init(int bufferSize)
{
    m_buffer = new uint8_t[bufferSize];
    m_bufferSize = bufferSize;

    m_pFront = m_buffer;
    m_pRear = m_buffer;
    m_pEndOfBuffer = m_buffer + bufferSize;
}

RingBuffer::RingBuffer()
{
    Init(DEFAULT_BUFFER_SIZE);
}

RingBuffer::RingBuffer(int size)
{
    if (size <= 1)
        Init(DEFAULT_BUFFER_SIZE);
    else
        Init(size);
}

//void RingBuffer::Resize(int size)
//{
//    if (size <= 1)
//        return;
//
//    uint8_t* newBuffer = new uint8_t[size];
//    m_bufferSize = size;
//
//    Dequeue(newBuffer, size);
//
//    m_buffer = newBuffer;
//    m_pFront = newBuffer;
//    m_pRear = newBuffer;
//    m_pEndOfBuffer = newBuffer + size;
//}

int RingBuffer::GetUseSize()
{
    if (m_pRear >= m_pFront)
        return (int)(m_pRear - m_pFront);

    int size1 = (int)(m_pEndOfBuffer - m_pFront);
    int size2 = (int)(m_pRear - m_buffer);

    return size1 + size2;
}

int RingBuffer::GetFreeSize()
{
    int size1, size2;

    if (m_pFront > m_pRear)
    {
        return (int)(m_pFront - m_pRear) - 1;
    }
    else if (m_pFront == m_pRear)
    {
        return m_bufferSize - 1;
    }
    else
    {
        size1 = (int)(m_pEndOfBuffer - m_pRear);
        size2 = (int)(m_pFront - m_buffer);

        return size1 + size2 - 1;
    }
}

int RingBuffer::DirectEnqueueSize()
{
    int size;

    if (m_pRear >= m_pFront)
    {
        size = (int)(m_pEndOfBuffer - m_pRear);

        if (m_pFront == m_buffer)
            return size - 1;

        return size;
    }
    else
    {
        size = (int)(m_pFront - m_pRear) - 1;

        return size;
    }
}

int RingBuffer::DirectDequeueSize()
{
    int size;

    if (m_pFront <= m_pRear)
    {
        size = (int)(m_pRear - m_pFront);
        return size;
    }
    else
    {
        size = (int)(m_pEndOfBuffer - m_pFront);
        return size;
    }
}

int RingBuffer::Enqueue(const void* data, int size)
{
    int freeSize = GetFreeSize();
    if (size > freeSize)
        size = freeSize;

    uint8_t* src = (uint8_t*)data;
    const int directEnqueueSize = DirectEnqueueSize();
    int remainSize;

    if (size > directEnqueueSize)
    {
        remainSize = size;

        memcpy(m_pRear, src, directEnqueueSize);
        src += directEnqueueSize;
        remainSize -= directEnqueueSize;

        m_pRear = m_buffer;
        memcpy(m_pRear, src, remainSize);
        m_pRear += remainSize;
    }
    else
    {
        memcpy(m_pRear, src, size);
        m_pRear += size;

        if (m_pRear == m_pEndOfBuffer)
            m_pRear = m_buffer;
    }

    return size;
}

int RingBuffer::Dequeue(void* out_data, int size)
{
    const int useSize = GetUseSize();
    if (size > useSize)
        size = useSize;

    uint8_t* dst = (uint8_t*)out_data;
    const int directDequeueSize = DirectDequeueSize();
    int remainSize;

    if (size > directDequeueSize)
    {
        remainSize = size;

        memcpy(dst, m_pFront, directDequeueSize);
        dst += directDequeueSize;
        remainSize -= directDequeueSize;

        m_pFront = m_buffer;
        memcpy(dst, m_pFront, remainSize);
        m_pFront += remainSize;
    }
    else
    {
        memcpy(dst, m_pFront, size);
        m_pFront += size;

        if (m_pFront == m_pEndOfBuffer)
            m_pFront = m_buffer;
    }

    return size;
}

int RingBuffer::Peek(void* out_data, int size)
{
    const int useSize = GetUseSize();
    if (size > useSize)
        size = useSize;

    uint8_t* dst = (uint8_t*)out_data;
    uint8_t* src = m_pFront;
    const int directDequeueSize = DirectDequeueSize();
    int remainSize;

    if (size > directDequeueSize)
    {
        remainSize = size;

        memcpy(dst, src, directDequeueSize);
        dst += directDequeueSize;
        remainSize -= directDequeueSize;

        src = m_buffer;
        memcpy(dst, src, remainSize);        
    }
    else
    {
        memcpy(dst, src, size);
    }

    return size;
}

int RingBuffer::MoveRear(int size)
{
    const int freeSize = GetFreeSize();
    if (size > freeSize)
        size = freeSize;

    const int directEnqueueSize = DirectEnqueueSize();
    int remainSize;

    if (size > directEnqueueSize)
    {
        remainSize = size;

        m_pRear += directEnqueueSize;
        remainSize -= directEnqueueSize;

        m_pRear = m_buffer;
        m_pRear += remainSize;
    }
    else // size <= directEnqueueSize
    {
        m_pRear += size;

        if (m_pRear == m_pEndOfBuffer)
            m_pRear = m_buffer;
    }

    return size;
}

int RingBuffer::MoveFront(int size)
{
    const int useSize = GetUseSize();
    if (size > useSize)
        size = useSize;

    int remainSize;
    const int directDequeueSize = DirectDequeueSize();

    if (size > directDequeueSize)
    {
        remainSize = size;

        m_pFront += directDequeueSize;
        remainSize -= directDequeueSize;

        m_pFront = m_buffer;
        m_pFront += remainSize;
    }
    else
    {
        m_pFront += size;

        if (m_pFront == m_pEndOfBuffer)
            m_pFront = m_buffer;
    }

    return size;
}
