
#include "webrtc/common_audio/vad/include/webrtc_vad.h"

#include <cassert> // assert
#include <iostream> // std::cout
#include <string> // std::string
#include <vector> // std::vector
#include <fstream> // std::ifstream

#include "RingBuffer.h"


void vad_free(void* pVoid)
{
  VadInst* handle = (VadInst*)pVoid;
  WebRtcVad_Free(handle);
}

void* vad_create()
{
  VadInst *handle = WebRtcVad_Create();
  return (void*)handle;
}

bool vad_init(void* pVoid)
{
    VadInst* handle = (VadInst*)pVoid;
    if (WebRtcVad_Init(handle)) 
    {
        return false;
    }
    return true;
}

bool vad_set_mode(void* pVoid, long mode)
{
    VadInst* handle = (VadInst*)pVoid;
    if (WebRtcVad_set_mode(handle, mode)) 
    {
        return false;
    }
    return true;
}

bool valid_rate_and_frame_length(long rate, long frame_length)
{
    if (WebRtcVad_ValidRateAndFrameLength(rate, frame_length)) 
    {
        return false;
    } 
    return true;
}

bool vad_process(void* pVoid, long sampleRate, const char* buf, long frame_length)
{
    VadInst* handle = (VadInst*)pVoid;
    assert( frame_length % 2 == 0 );
    int result =  WebRtcVad_Process(handle, sampleRate, (int16_t*)buf, frame_length/2);
    if( result > 0 )
        return true;
    else
        return false;
}


bool read_wave(const char* fileName, unsigned int& sampleRate, char** audioData, uint64_t& audioLength)
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

    uint64_t bufSize = fileSize - 44;
    audioLength = bufSize;
    *audioData = new char[bufSize];
    fseek( pf, 44, SEEK_SET );
    fread( *audioData, 1, bufSize, pf );

    fclose( pf );

    return true;
}

void write_wave(const char* fileName, const char* audioData, uint64_t audioLength, int sample_rate)
{
    std::cout<<"write wav: " <<fileName<<std::endl;
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
        if( nullptr != bytes )
        {
            delete []bytes;
            bytes = nullptr;
        }
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

std::vector<Segment> vad_collector(void* vad, unsigned int sample_rate, unsigned int frame_duration_ms,
        unsigned int padding_duration_ms, std::vector<Frame>& frames)
{
    // Given a webrtcvad.Vad and a source of audio frames, yields only
    // the voiced audio.

    // Uses a padded, sliding window algorithm over the audio frames.
    // When more than 90% of the frames in the window are voiced (as
    // reported by the VAD), the collector triggers and begins yielding
    // audio frames. Then the collector waits until 90% of the frames in
    // the window are unvoiced to detrigger.

    // The window is padded at the front and back to provide a small
    // amount of silence or the beginnings/endings of speech around the
    // voiced frames.

    // Arguments:

    // sample_rate - The audio sample rate, in Hz.
    // frame_duration_ms - The frame duration in milliseconds.
    // padding_duration_ms - The amount to pad the window, in milliseconds.
    // vad - An instance of webrtcvad.Vad.
    // frames - a source of audio frames (sequence or generator).

    // Returns: A generator that yields PCM audio data.

    int num_padding_frames = padding_duration_ms / frame_duration_ms;
    // We have two states: TRIGGERED and NOTTRIGGERED. We start in the
    // NOTTRIGGERED state.
    bool triggered = false;

    std::vector<Segment> segments;

    std::vector<Frame> voiced_frames;
    Buffers::RingBuffer<Frame> ring_buffer(num_padding_frames);

    for( Frame& frame : frames )
    {
        frame.silence = !vad_process(vad, sample_rate, frame.bytes, frame.length);

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
                std::cout<<"+"<<ring_buffer[0].timestamp;
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
                std::cout<<"-"<<frame.timestamp + frame.duration;
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

int vadSplit( const char* fileName ) 
{
    int aggressiveness = 0; // 0, 1, 2, 4
    void* vad = vad_create();
    vad_init( vad );
    vad_set_mode( vad, aggressiveness );
    unsigned int sampleRate = 0;
    char* audioData = nullptr;
    uint64_t audioLength = 0;
    if( !read_wave( fileName, sampleRate, &audioData, audioLength ))
    {
        std::cout<<"Failed to read wav file"<<std::endl;
        return -1;
    }
    auto frames = frame_generator(30, audioData, audioLength, sampleRate);
    auto segments = vad_collector(vad, sampleRate, 30, 300, frames);
    int num = 0;
    for( auto& segment : segments )
    {
        char path[256];
        snprintf(path, sizeof(path), "chunk-%02d.wav", num++);
        write_wave(path, segment.start, segment.length, sampleRate);
    }
    if( nullptr != audioData )
    {
        delete []audioData;
        audioData = nullptr;
    }
        
    return 0;
}
