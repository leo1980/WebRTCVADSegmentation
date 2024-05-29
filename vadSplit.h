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

#ifndef _VAD_SPLIT_H_
#define _VAD_SPLIT_H_

#include <vector>

// offset to data pointer
struct VadSegment
{
    unsigned int offset;
    unsigned int length;
    float start;
    float end;
    VadSegment( unsigned int offset, unsigned int len, float startTime, float endTime )
    : offset( offset )
    , length( len )
    , start( startTime )
    , end( endTime )
    {
    }
};

/**
* @aggressiveness 
* it is an integer between 0 and 3. 
* 0 is the least aggressive about filtering out non-speech, 3 is the most aggressive
* @outputFmt
*     0: pcm
*     1: wav
*     -1: don't write output file
*/
int vadSplit( const char* fileName, std::vector<VadSegment>& segment, int outputFmt = -1, int aggressiveness = 2 );

int vadSplit( const char* audioData, uint64_t audioLength, unsigned int sampleRate,  std::vector<VadSegment>& segment, int outputFmt = -1, int aggressiveness = 2 );

#endif // _VAD_SPLIT_H_
