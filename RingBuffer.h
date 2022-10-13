/**
 * MIT License
 *  
 * Copyright (c) 2022 360Converter - Leo Huang
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once 

#include <stdexcept>

using namespace std;

typedef unsigned int uint32;
typedef unsigned long uint64;

namespace Buffers
{

template<class T>
class RingBuffer 
{
public:
    // Constructor
    RingBuffer() = default;

    // Constructor
    explicit RingBuffer(unsigned int size);

    // Destructor
    ~RingBuffer();

    // Iterator class
    class iterator;

    // Add element to Ring Buffer
    void push_back(T value) noexcept;

    // Get size of the Ring Buffer
    uint32 capacity() const noexcept;

    // Clear element
    void clear() noexcept;

    // Get front element 
    const T& front() const;

    // Get back element
    const T& back() const;

    // How many elemented has been inserted
    uint64 size() const noexcept;

    // Access element at index
    T& operator[](unsigned int index);

    // Access element at index and return const reference
    const T& operator[](unsigned int index) const;

    // The iterate of the first element 
    iterator begin() const noexcept
    {
        if( m_count == 0 )
            return end();
        return iterator(m_tail, *this);
    }

    // The iterate of the last slot of buffer which will never be accessed
    iterator end() const noexcept
    {
        return iterator(m_capacity, *this);
    }

private:
    uint32 m_capacity;
    T* m_data;
    uint32 m_head;
    uint32 m_tail;
    uint64 m_count;
};

template<class T>
class RingBuffer<T>::iterator 
{
public:
    // Constructor
    iterator(uint32 position, const RingBuffer& ringBuffer)
        : m_current(position)
        , m_ringBuffer(ringBuffer) 
        , m_step(0)
    {
    }

    // Overload Operator Postfix ++
    iterator& operator++(int) 
    {
        m_step++;
        m_current++;
        if( m_current >= m_ringBuffer.m_capacity )
            m_current = 0;
        if( m_step >= m_ringBuffer.m_capacity || m_step >= m_ringBuffer.m_count )
        {
            m_current = m_ringBuffer.m_capacity;
        }
        return *this;
    }

    // Overload operator prefix ++
    iterator& operator++() noexcept
    {
        m_step++;
        m_current++;
        if( m_current >= m_ringBuffer.m_capacity )
            m_current = 0;
        if( m_step >= m_ringBuffer.m_capacity || m_step >= m_ringBuffer.m_count )
        {
            m_current = m_ringBuffer.m_capacity;
        }
        return *this;
    }

    // Overload de-reference operator
    const T& operator*() const noexcept
    {
        return m_ringBuffer[m_current];
    }

    // Overload operator !=
    bool operator!=(const iterator& other) const 
    {
        return (m_current != other.m_current);
    }

private:
    uint32 m_current;
    const RingBuffer& m_ringBuffer;
    uint32 m_step;
};

template<class T>
RingBuffer<T>::RingBuffer(unsigned int size) 
    : m_capacity(size)
    , m_head(0)
    , m_tail(0)
    , m_data(nullptr) 
    , m_count( 0 )
{
    m_data = new T[m_capacity+1];
}

template<class T>
RingBuffer<T>::~RingBuffer() 
{
    if (m_data) 
    {
        delete[] m_data;
        m_data = nullptr;
    }
}

template<class T>
void RingBuffer<T>::push_back(T value) noexcept
{
    m_data[m_head] = value;
    m_count++;
    m_head++;
    if(m_head >= m_capacity ) 
    {
        m_head = 0;
    }

    if( m_count > m_capacity )
    {
        m_tail++;
        if(m_tail >= m_capacity) 
        {
            m_tail = 0;
        }
    }
}

template<class T>
uint32 RingBuffer<T>::capacity() const noexcept
{
    return m_capacity;
}

template<class T>
void RingBuffer<T>::clear() noexcept
{
    m_head = 0;
    m_tail = 0;
    m_count = 0;
}

template<class T>
const T& RingBuffer<T>::front() const
{
    if( m_count == 0 )
        throw out_of_range("Beyound boundary");
    return m_data[m_tail];
}

template<class T>
const T& RingBuffer<T>::back() const
{
    if( m_count == 0 )
        throw out_of_range("Beyound boundary");
    return m_data[m_head];
}

template<class T>
uint64 RingBuffer<T>::size() const noexcept
{
    return m_count;
}

template<class T>
T& RingBuffer<T>::operator[](unsigned int index)
{
    if( index >= m_capacity )
        throw out_of_range("Beyound boundary");
    return m_data[index];
}

template<class T>
const T& RingBuffer<T>::operator[](unsigned int index) const
{
    if( index >= m_capacity )
        throw out_of_range("Beyound boundary");
    return m_data[index];
}


} // namepsace Buffers
