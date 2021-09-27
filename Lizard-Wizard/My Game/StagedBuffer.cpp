#include "StagedBuffer.h"
#include <iostream>

StagedBuffer::StagedBuffer(usize capacity) {
    m_Pointer = new u8[capacity];
    m_Capacity = capacity;
    m_Length = 0;
}
 
StagedBuffer::~StagedBuffer(){
    delete[] m_Pointer;
    m_Capacity = 0;
    m_Length = 0;
}

/// <summary>
/// SAFETY: requested_reset must be a valid number between 0 and m_Capacity
/// </summary>
/// <param name="requested_reset"></param>
void StagedBuffer::Reset(usize requested_reset) {
    if (requested_reset <= m_Capacity) {
        m_Length = requested_reset;
    } else {
        ABORT("Requested buffer reset would overflow!\n");
    }
}

/// <summary>
/// Allocates contiguous memory for some type T
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="requested_size"></param>
/// <returns></returns>
u8* StagedBuffer::Alloc(usize requested_size) {
    usize would_be_offset = m_Length + requested_size;

    if (would_be_offset > m_Capacity) { // bad allocation
        char buffer[256];
        sprintf(
            buffer,
            "Requested allocation of %llu bytes would offset buffer to %llu bytes when buffer can only hold %llu bytes!\n",
            requested_size, would_be_offset, m_Capacity
        );

        ABORT(buffer);
    } else { // actually allocate
        u8* pointer = (u8*)(m_Pointer + m_Length);
        m_Length += requested_size;
        return pointer;
    }
}
