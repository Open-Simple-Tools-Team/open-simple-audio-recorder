//
//  osaudiorecorder.h
//  osaudiorecorder-core-osx
//
//  Created by Marcos Ortega on 9/8/20.
//

#ifndef osaudiorecorder_h
#define osaudiorecorder_h

#include <stdio.h>	//FILE*

typedef char OSARBool;
#define TRUE  1
#define FALSE 0

typedef struct STOSAudioRecorder_ {
	void* opaque;
} STOSAudioRecorder;

//--------
//- Encoder plugin callbacks
//--------

typedef void*     (*OSAudioRecorderEncoderMethod_start)(FILE* dst, int channels, int bitsPerSample, int samplerate, int blockAlign, int samplesTotal);
typedef OSARBool  (*OSAudioRecorderEncoderMethod_feed)(void* userData, FILE* dst, void* samples, long samplesBytes, long samplesCount);
typedef OSARBool  (*OSAudioRecorderEncoderMethod_end)(void* userData, FILE* dst);

//--------
//- Engine
//--------

OSARBool	OSAudioRecorder_init(STOSAudioRecorder* obj); 
void		OSAudioRecorder_release(STOSAudioRecorder* obj);

//Must be manually called, few times per second
void		OSAudioRecorder_tick(STOSAudioRecorder* obj);

//----------------
//- Recorder (WAV)
//----------------

OSARBool	OSAudioRecorder_start(STOSAudioRecorder* obj, const char* wavFilepath, const int buffersPerSec, const OSARBool keepAvgsOfSamples); 
OSARBool	OSAudioRecorder_pause(STOSAudioRecorder* obj);
OSARBool	OSAudioRecorder_resume(STOSAudioRecorder* obj);
OSARBool	OSAudioRecorder_stop(STOSAudioRecorder* obj);
OSARBool	OSAudioRecorder_reset(STOSAudioRecorder* obj);

OSARBool	OSAudioRecorder_isActive(STOSAudioRecorder* obj);		//recording or paused
OSARBool	OSAudioRecorder_isRecording(STOSAudioRecorder* obj);
OSARBool	OSAudioRecorder_isPaused(STOSAudioRecorder* obj);

//current or last recording stats
unsigned long	OSAudioRecorder_samplesCount(STOSAudioRecorder* obj);
unsigned long	OSAudioRecorder_samplesBytes(STOSAudioRecorder* obj);
float			OSAudioRecorder_samplesSecs(STOSAudioRecorder* obj);

//Avg of samples from -1 to 1 (warning: overflows if not manually reset preiodically)
float			OSAudioRecorder_samplesRelAvg(STOSAudioRecorder* obj, const OSARBool resetAvg);

//--------------
//- Player (WAV)
//--------------

OSARBool	OSAudioRecorder_playerLoad(STOSAudioRecorder* obj, const char* filepath, const int buffersPerSec);
OSARBool	OSAudioRecorder_playerPlay(STOSAudioRecorder* obj); 
OSARBool	OSAudioRecorder_playerPause(STOSAudioRecorder* obj);
OSARBool	OSAudioRecorder_playerStop(STOSAudioRecorder* obj);

OSARBool	OSAudioRecorder_playerIsLoaded(STOSAudioRecorder* obj);
OSARBool	OSAudioRecorder_playerIsPaused(STOSAudioRecorder* obj);

OSARBool	OSAudioRecorder_playerSeekRelPos(STOSAudioRecorder* obj, const float relPos);

float		OSAudioRecorder_playerRelProgress(STOSAudioRecorder* obj);
float		OSAudioRecorder_playerSecsTotal(STOSAudioRecorder* obj);

//------------
//- Encoder (from WAV to ...)
//------------

OSARBool	OSAudioRecorder_encoderAdd(STOSAudioRecorder* obj, const char* formatId, OSAudioRecorderEncoderMethod_start methodStart, OSAudioRecorderEncoderMethod_feed methodFeed, OSAudioRecorderEncoderMethod_end methodEnd);

OSARBool	OSAudioRecorder_encoderStart(STOSAudioRecorder* obj, const char* filepathSrc, const char* filepathDst, const char* outputFormatId);
OSARBool	OSAudioRecorder_encoderIsLoaded(STOSAudioRecorder* obj);
float		OSAudioRecorder_encoderRelProgress(STOSAudioRecorder* obj);	//Never 1.0f unless is completed
OSARBool	OSAudioRecorder_encoderFinish(STOSAudioRecorder* obj);		//Cancel

#endif /* osaudiorecorder_h */

//end-of-file
