#include <iostream> // std::cout
#include <string> // std::string
#include <vector> // std::vector

#include "RingBuffer.h"
#include "vadSplit.h"

void testRingBuffer1()
{
    Buffers::RingBuffer<int> ringBuffer(8);
    for( int i = 0; i < 4; ++i )
    {
        ringBuffer.push_back( i );
    }

    // Print RingBuffer with iterator
    for(auto it = ringBuffer.begin(); it != ringBuffer.end(); it++) {
        cout << *it << endl;
    }
    cout<<"---------------------------"<<std::endl;
    Buffers::RingBuffer<int> ringBuffer1(8);
    for( int i = 0; i < 10; ++i )
    {
        ringBuffer1.push_back( i );
    }

    // Print RingBuffer with iterator
    for(auto it = ringBuffer1.begin(); it != ringBuffer1.end(); it++) {
        cout << *it << endl;
    }
    cout<<"---------------------------"<<std::endl;
    Buffers::RingBuffer<int> ringBuffer2(8);
    for( int i = 0; i < 127; ++i )
    {
        ringBuffer2.push_back( i );
    }

    // Print RingBuffer with iterator
    for(auto it = ringBuffer2.begin(); it != ringBuffer2.end(); it++) {
        cout << *it << endl;
    }
    cout<<"---------------------------"<<std::endl;
    Buffers::RingBuffer<std::string> ringBuffer3(8);
    for( int i = 0; i < 10; ++i )
    {
        ringBuffer3.push_back( std::string("testing") + std::to_string( i) );
    }

    // Print RingBuffer with iterator
    for(auto it = ringBuffer3.begin(); it != ringBuffer3.end(); it++) {
        cout << *it << endl;
    }
}

int main( int argc, char* argv[])
{
    std::cout<<"Test vadSplit"<<std::endl;
    //testRingBuffer();
    //testRingBuffer1();

    vadSplit( argv[1] );
        
    return 0;
}
