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

#include "webrtc/common_audio/vad/include/webrtc_vad.h"
#include "RingBuffer.h"

#include <cassert> // assert
#include <iostream> // std::cout
#include <string> // std::string
#include <vector> // std::vector
#include <fstream> // std::ifstream

bool vadProcess(VadInst* handle, long sampleRate, const char* buf, long frame_length)
{
    assert( frame_length % 2 == 0 );
    int result =  WebRtcVad_Process(handle, sampleRate, (int16_t*)buf, frame_length/2);
    if( result > 0 )
        return true;
    else
        return false;
}

bool readWavFile(const char* fileName, unsigned int& sampleRate, char** audioData, uint64_t& audioLength)
{
    // TODO: read sample rate
    sampleRate = 16000;
    FILE *pf = fopen( fileName, "rb" );
    if( nullptr == pf )
        return false;

    // Get file size
    std::ifstream inFile(fileName, std::ios::binary);
    inFile.seekg(0, std::ios::end);
    uint64_t fileSize = inFile.tellg();

    // 44 = wav header size
    uint64_t bufSize = fileSize - 44;
    audioLength = bufSize;
    *audioData = new char[bufSize];
    fseek( pf, 44, SEEK_SET );
    fread( *audioData, sizeof( char ), bufSize, pf );

    fclose( pf );

    return true;
}

bool writeRawAudioFile(const char* fileName, const char* audioData, uint64_t audioLength, int sample_rate)
{
    FILE *pf = fopen( fileName, "wb" );
    if( nullptr == pf )
    {
        return false;
    }
    fwrite( audioData, sizeof( char ), audioLength, pf );
    fclose( pf );

    return true;
}

void writeWord( FILE* pf, unsigned int value, unsigned size )
{
    for (; size; --size, value >>= 8)
    {
        char c = static_cast<char>(value & 0xFF);
        fwrite( &c, sizeof( char ), 1, pf );
    }
}

bool writeWavFile(const char* fileName, const char* audioData, uint64_t audioLength, int sampleRate)
{
    FILE *pf = fopen( fileName, "wb" );
    if( nullptr == pf )
    {
        return false;
    }
    unsigned int channel = 1;
    unsigned int bitsPerSample = 16;
    unsigned int strideSize = sampleRate * bitsPerSample * channel / 8; // (Sample Rate * BitsPerSample * Channels) / 8
    unsigned int blockSize = bitsPerSample * channel / 8; // ( BitsPerSample * Channels) / 8
    fwrite( "RIFF", 1, 4, pf );
    writeWord( pf, static_cast<unsigned int>(audioLength) + 44, 4 ); // write file size
    fwrite( "WAVE", 1, 4, pf );
    fwrite( "fmt ", 1, 4, pf );
    writeWord( pf, 16, 4 );  // no extension data
    writeWord( pf, 1, 2 );  // PCM - integer samples
    writeWord( pf, channel, 2 );  // two channels (stereo file)
    writeWord( pf, sampleRate, 4 );  // samples per second (Hz)
    writeWord( pf, strideSize, 4 );  
    writeWord( pf, blockSize, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
    writeWord( pf, bitsPerSample, 2 );  // number of bits per sample (use a multiple of 8)
    fwrite( "data", 1, 4, pf );
    writeWord( pf, static_cast<unsigned int>(audioLength), 4 ); // write data size
    fwrite( audioData, sizeof( char ), audioLength, pf );
    fclose( pf );

    return true;
}

// To avoid memory copy, Frame just store start position of audio and length
class Frame
{
public:
    Frame()
    : silence( false )
    , bytes( nullptr )
    , length( 0 )
    , timestamp( 0.0f )
    , duration( 0.0f )
    {
    }

    Frame( const char* bytes, int length, float timestamp, float duration )
    : silence( false )
    , bytes( bytes )
    , length( length )
    , timestamp( timestamp )
    , duration( duration )
    {
    }

    ~Frame()
    {
    }

    Frame& operator=( const Frame& frame )
    {
        if( this == &frame )
            return *this;
        bytes = frame.bytes;
        length = frame.length;
        timestamp = frame.timestamp;
        duration = frame.duration;
        silence = frame.silence;

        return *this;
    }

    Frame( const Frame& frame )
    {
        bytes = frame.bytes;
        length = frame.length;
        timestamp = frame.timestamp;
        duration = frame.duration;
        silence = frame.silence;
    }

public:
    bool silence;
    const char* bytes;
    size_t length;
    float timestamp;
    float duration;
};

std::vector<Frame> frame_generator(int frame_duration_ms, const char* audio, unsigned int audioLength, unsigned int sample_rate)
{
    std::vector<Frame> frames;
    int n = int(sample_rate * (frame_duration_ms / 1000.0) * 2);
    int offset = 0;
    float timestamp = 0.0;
    float duration = (float(n) / sample_rate) / 2.0;
    while( offset + n < audioLength )
    {
        Frame frame(audio + offset, n, timestamp, duration);
        frames.push_back( frame );
        timestamp += duration;
        offset += n;
    }

    return frames;
}

struct Segment
{
    const char* start;
    unsigned int length;
};

std::vector<Segment> vadCollector(VadInst* handle, unsigned int sample_rate, unsigned int frame_duration_ms,
        unsigned int padding_duration_ms, std::vector<Frame>& frames)
{
    int num_padding_frames = padding_duration_ms / frame_duration_ms;

    // We have two states: TRIGGERED and NOTTRIGGERED. We start in the
    // NOTTRIGGERED state.
    bool triggered = false;

    std::vector<Segment> segments;
    std::vector<Frame> voiced_frames;
    Buffers::RingBuffer<Frame> ring_buffer(num_padding_frames);

    for( Frame& frame : frames )
    {
        bool speech = vadProcess(handle, sample_rate, frame.bytes, frame.length);
        if( speech )
        {
            std::cout<<"1";
        }
        else
        {
            std::cout<<"0";
        }
        frame.silence = !speech;

        if( !triggered )
        {
            ring_buffer.push_back(frame);
            unsigned int num_voiced = 0;
            for(auto it = ring_buffer.begin(); it != ring_buffer.end(); it++) 
            {
                if( !(*it).silence )
                    num_voiced++;
            }

            // If we're NOTTRIGGERED and more than 90% of the frames in
            // the ring buffer are voiced frames, then enter the
            // TRIGGERED state.
            if ( num_voiced > 0.9 * ring_buffer.capacity())
            {
                triggered = true;
                std::cout<<"+("<<ring_buffer.front().timestamp<<")";

                // We want to yield all the audio we see from now until
                // we are NOTTRIGGERED, but we have to start with the
                // audio that's already in the ring buffer.
                for(auto it = ring_buffer.begin(); it != ring_buffer.end(); it++) 
                {
                    voiced_frames.push_back( *it );
                }
                ring_buffer.clear();
            }
        }
        else
        {
            // We're in the TRIGGERED state, so collect the audio data
            // and add it to the ring buffer.
            voiced_frames.push_back(frame);
            ring_buffer.push_back(frame);
            unsigned int num_unvoiced = 0;
            for(auto it = ring_buffer.begin(); it != ring_buffer.end(); it++) 
            {
                if( (*it).silence )
                    num_unvoiced++;
            }

            // If more than 90% of the frames in the ring buffer are
            // unvoiced, then enter NOTTRIGGERED and yield whatever
            // audio we've collected.
            if( num_unvoiced > 0.9 * ring_buffer.capacity())
            {
                std::cout<<"-("<<frame.timestamp + frame.duration<<")";
                triggered = false;
                Segment segment;
                segment.start = voiced_frames[0].bytes;
                segment.length = voiced_frames.size() * voiced_frames[0].length;
                segments.push_back( segment );
                ring_buffer.clear();
                voiced_frames.clear();
            }
        }
    }
    std::cout<<std::endl;

    // If we have any leftover voiced audio when we run out of input,
    // yield it.
    if( voiced_frames.size() > 0 )
    {
        Segment segment;
        segment.start = voiced_frames[0].bytes;
        segment.length = voiced_frames.size() * voiced_frames[0].length;
        segments.push_back( segment );
    }

    return segments;
}

int vadSplit( const char* fileName, int outputFmt, int aggressiveness/* = 2*/ ) 
{
    VadInst *vad = WebRtcVad_Create();
    if (WebRtcVad_Init(vad)) 
    {
        return -1;
    }

    if (WebRtcVad_set_mode(vad, aggressiveness)) 
    {
        return -1;
    }
    unsigned int sampleRate = 0;
    char* audioData = nullptr;
    uint64_t audioLength = 0;
    if( !readWavFile( fileName, sampleRate, &audioData, audioLength ))
    {
        std::cout<<"Failed to read wav file"<<std::endl;
        return -2;
    }
    auto frames = frame_generator(30, audioData, audioLength, sampleRate);
    auto segments = vadCollector(vad, sampleRate, 30, 300, frames);
    int num = 0;
    for( auto& segment : segments )
    {
        char path[256];
        if( outputFmt == 0 )
        {
            snprintf(path, sizeof(path), "chunk-%02d.pcm", num++);
            std::cout<<"write audio: " <<path<<std::endl;
            writeRawAudioFile(path, segment.start, segment.length, sampleRate);
        }
        else
        {
            snprintf(path, sizeof(path), "chunk-%02d.wav", num++);
            std::cout<<"write audio: " <<path<<std::endl;
            writeWavFile(path, segment.start, segment.length, sampleRate);
        }
    }
    if( nullptr != audioData )
    {
        delete []audioData;
        audioData = nullptr;
    }
        
    return segments.size();
}
