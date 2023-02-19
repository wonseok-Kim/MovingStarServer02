#pragma once

#include <stdint.h>

class RingBuffer
{
  public:
    RingBuffer();
    RingBuffer(int size);

    //void Resize(int size);

    int GetBufferSize()
    {
        return m_bufferSize;
    }

    int GetUseSize();
    int GetFreeSize();

    int DirectEnqueueSize();
    int DirectDequeueSize();

    int Enqueue(const void* data, int size);
    int Dequeue(void* out_data, int size);

    int Peek(void* out_data, int size);

    int MoveRear(int size);
    int MoveFront(int size);

    void ClearBuffer()
    {
        m_pFront = m_buffer;
        m_pRear = m_buffer;
        // m_pFront = m_pRear;
    }

    uint8_t* GetFrontPtr()
    {
        return m_pFront;
    }
    uint8_t* GetRearPtr()
    {
        return m_pRear;
    }

  private:
    void Init(int size);

  private:
    static constexpr int DEFAULT_BUFFER_SIZE = 10000;

    uint8_t* m_pFront;
    uint8_t* m_pRear;
    uint8_t* m_pEndOfBuffer;

    int m_bufferSize;
    uint8_t* m_buffer;
};
