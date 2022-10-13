# WebRTC VAD Segmentation

Split audio into a series of short audios by detecting silence. Input file must be 16-bit mono PCM audio, sampled at 16000Hz.

Inspired by 
https://github.com/wiseman/py-webrtcvad

## Usage
``` c++
#include "RingBuffer.h"
#include "vadSplit.h"

// 0, 1, 2, 3
int aggressiveness = 1; 

// 0: pcm, 1: wav
int outFmt = 1;

const char* wavFile = "path/to/wav";

vadSplit( wavFile, outFmt, aggressiveness );
```
Aggressivenessh is an integer between 0 and 3. 0 is the least aggressive about filtering out non-speech, 3 is the most aggressive.
The WebRTC VAD only accepts 16-bit mono PCM audio, sampled at 8000, 16000, 32000 or 48000Hz.

## Build 
``` bash
$> mkdir build
$> cmake ..
$> make
```

## Play Raw Audio File
``` bash
$> ffplay -f s16le -ac 1 -ar 16000 chunk-01.wav
```

## TODO
- add switch to debug information
- add check for audio bits per sample, channel and samplerate
- add reading sample rate from wav file
- add unit tests
