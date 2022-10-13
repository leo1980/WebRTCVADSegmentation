/**
 * Copyright (c) 2022 360Converter - Leo Huang 
 *
 * See LICENSE for clarification regarding multiple authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream> // std::cout

#include "RingBuffer.h"
#include "vadSplit.h"

int main( int argc, char* argv[])
{
    if( argc < 2 )
    {
        std::cout<<"Error: at least wav file is needed"<<std::endl;
        std::cout<<"Usage: "<<std::endl;
        std::cout<<"    testVadSplit wav_file [aggresiveness] [output format]"<<std::endl;
        return -1;
    }

    // 0, 1, 2, 3
    int aggressiveness = 1; 

    // 0: pcm, 1: wav
    int outFmt = 1;
    if( argc > 2 )
        aggressiveness = atoi( argv[2] );
    if( argc > 3 )
        outFmt = atoi( argv[3] );
                            
    vadSplit( argv[1], outFmt, aggressiveness );
        
    return 0;
}
