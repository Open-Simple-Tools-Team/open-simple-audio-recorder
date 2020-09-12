//
//  nixtla.h
//  NixtlaAudioLib
//
//  Created by Marcos Ortega on 11/02/14.
//  Copyright (c) 2014 NIBSA. All rights reserved.
//
//  This entire notice must be retained in this source code.
//  This source code is under LGLP v2.1 Licence.
//
//  This software is provided "as is", with absolutely no warranty expressed
//  or implied. Any use is at your own risk.
//
//  Latest fixes enhancements and documentation at https://github.com/nicaraguabinary/nixtla-audio
//

#ifndef NixtlaAudioLib_nixtla_h
#define NixtlaAudioLib_nixtla_h

#ifdef __cplusplus
extern "C" {
#endif

//+++++++++++++++++++
//++ PUBLIC HEADER ++
//+++++++++++++++++++
	
//-------------------------------
//-- BASIC DEFINITIONS
//-------------------------------

//Data types
typedef unsigned char 		NixBOOL;	//BOOL, Unsigned 8-bit integer value
typedef unsigned char 		NixBYTE;	//BYTE, Unsigned 8-bit integer value
typedef char 				NixSI8;		//NIXSI8, Signed 8-bit integer value
typedef	short int 			NixSI16;	//NIXSI16, Signed 16-bit integer value
typedef	int 				NixSI32;	//NIXSI32, Signed 32-bit integer value
typedef	long long 			NixSI64;	//NixSI64, Signed 64-bit integer value
typedef unsigned char 		NixUI8;		//NixUI8, Unsigned 8-bit integer value
typedef	unsigned short int 	NixUI16;	//NixUI16, Unsigned 16-bit integer value
typedef	unsigned int 		NixUI32;	//NixUI32, Unsigned 32-bit integer value
typedef	unsigned long long	NixUI64;	//NixUI64[n], Unsigned 64-bit array—n is the number of array elements
typedef float				NixFLOAT;	//float

#define NIX_FALSE			0
#define NIX_TRUE			1
	
//Capabilities mask
#define NIX_CAP_AUDIO_CAPTURE			1
#define NIX_CAP_AUDIO_STATIC_BUFFERS	2
#define NIX_CAP_AUDIO_SOURCE_OFFSETS	4

//Audio description
typedef enum ENNix_sampleFormat_ {
	ENNix_sampleFormat_none = 0,
	ENNix_sampleFormat_int,			//unsigned byte for 8-bits, signed short/int/long for 16/24/32/64 bits
	ENNix_sampleFormat_float
} ENNix_sampleFormat;
	
typedef struct STNix_audioDesc_ {
	NixUI8	samplesFormat;	//ENNix_sampleFormat
	NixUI8	channels;
	NixUI8	bitsPerSample;
	NixUI16	samplerate;
	NixUI16	blockAlign;
} STNix_audioDesc;

//-------------------------------
//-- ENGINES
//-------------------------------
	
//Engine object
typedef struct STNix_Engine_ {
	void* o; //Abstract objects pointer
} STNix_Engine;

//Engine status
typedef struct STNix_StatusDesc_ {
	//Sources
	NixUI16	countSources;			//All sources
	NixUI16	countSourcesReusable;	//Only reusable sources. Not-reusable = (countSources - countSourcesReusable);
	NixUI16	countSourcesAssigned;	//Only sources retained by ussers. Not-assigned = (countSources - countSourcesAssigned);
	NixUI16	countSourcesStatic;		//Only static sounds sources.
	NixUI16	countSourcesStream;		//Only stream sounds sources. Undefined-sources = (countSources - countSourcesStatic - countSourcesStream);
	//PLay buffers
	NixUI16	countPlayBuffers;		//All play-buffers
	NixUI32	sizePlayBuffers;		//Bytes of all play-buffers
	NixUI32	sizePlayBuffersAtSW;	//Only bytes of play-buffers that resides in nixtla's memory. sizeBuffersAtExternal = (sizeBuffers - sizeBuffersAtSW); //this includes Hardware-buffers
	//Record buffers
	NixUI16	countRecBuffers;		//All rec-buffers
	NixUI32	sizeRecBuffers;			//Bytes of all rec-buffers
	NixUI32	sizeRecBuffersAtSW;		//Only bytes of rec-buffers that resides in nixtla's memory. sizeBuffersAtExternal = (sizeBuffers - sizeBuffersAtSW); //this includes Hardware-buffers
} STNix_StatusDesc;

	
//Callbacks
typedef void (*PTRNIX_SourceReleaseCallback)(STNix_Engine* engAbs, void* userdata, const NixUI32 sourceIndex);
typedef void (*PTRNIX_StreamBufferUnqueuedCallback)(STNix_Engine* engAbs, void* userdata, const NixUI32 sourceIndex, const NixUI16 buffersUnqueuedCount);
typedef void (*PTRNIX_CaptureBufferFilledCallback)(STNix_Engine* engAbs, void* userdata, const STNix_audioDesc audioDesc, const NixUI8* audioData, const NixUI32 audioDataBytes, const NixUI32 audioDataSamples);

//Engine
NixBOOL		nixInit(STNix_Engine* engAbs, const NixUI16 pregeneratedSources);
void		nixFinalize(STNix_Engine* engAbs);
NixBOOL		nixContextIsActive(STNix_Engine* engAbs);
NixBOOL		nixContextActivate(STNix_Engine* engAbs);
NixBOOL		nixContextDeactivate(STNix_Engine* engAbs);
//void		nixGetContext(STNix_Engine* engAbs, void* dest); //"dest" must be a "ALCcontext**" or "SLEngineItf*"
void		nixGetStatusDesc(STNix_Engine* engAbs, STNix_StatusDesc* dest);
NixUI32		nixCapabilities(STNix_Engine* engAbs); //NIX_CAP_AUDIO_*
void		nixPrintCaps(STNix_Engine* engAbs);
void		nixTick(STNix_Engine* engAbs);

//Buffers
NixUI16		nixBufferWithData(STNix_Engine* engAbs, const STNix_audioDesc* audioDesc, const NixUI8* audioDataPCM, const NixUI32 audioDataPCMBytes);
NixBOOL		nixBufferSetData(STNix_Engine* engAbs, const NixUI16 buffIndex, const STNix_audioDesc* audioDesc, const NixUI8* audioDataPCM, const NixUI32 audioDataPCMBytes);
NixUI32		nixBufferRetainCount(STNix_Engine* engAbs, const NixUI16 buffIndex);
void		nixBufferRetain(STNix_Engine* engAbs, const NixUI16 buffIndex);
void		nixBufferRelease(STNix_Engine* engAbs, const NixUI16 buffIndex);
float		nixBufferSeconds(STNix_Engine* engAbs, const NixUI16 buffIndex);
STNix_audioDesc nixBufferAudioDesc(STNix_Engine* engAbs, const NixUI16 buffIndex);

//Sources
NixUI16		nixSourceAssignStatic(STNix_Engine* engAbs, NixUI8 lookIntoReusable, NixUI8 audioGroupIndex, PTRNIX_SourceReleaseCallback releaseCallBack, void* releaseCallBackUserData);
NixUI16		nixSourceAssignStream(STNix_Engine* engAbs, NixUI8 lookIntoReusable, NixUI8 audioGroupIndex, PTRNIX_SourceReleaseCallback releaseCallBack, void* releaseCallBackUserData, const NixUI16 queueSize, PTRNIX_StreamBufferUnqueuedCallback bufferUnqueueCallback, void* bufferUnqueueCallbackData);
NixUI32		nixSourceRetainCount(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourceRetain(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourceRelease(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourceSetRepeat(STNix_Engine* engAbs, const NixUI16 sourceIndex, const NixBOOL repeat);
NixUI32		nixSourceGetSamples(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixUI32		nixSourceGetBytes(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixFLOAT	nixSourceGetSeconds(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixFLOAT	nixSourceGetVoume(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourceSetVolume(STNix_Engine* engAbs, const NixUI16 sourceIndex, const float volume);
NixUI16		nixSourceGetBuffersCount(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixUI32		nixSourceGetOffsetSamples(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixUI32		nixSourceGetOffsetBytes(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourceSetOffsetSamples(STNix_Engine* engAbs, const NixUI16 sourceIndex, const NixUI32 offsetSamples);
void		nixSourcePlay(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixBOOL		nixSourceIsPlaying(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourcePause(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourceStop(STNix_Engine* engAbs, const NixUI16 sourceIndex);
void		nixSourceRewind(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixBOOL		nixSourceSetBuffer(STNix_Engine* engAbs, const NixUI16 sourceIndex, const NixUI16 bufferIndex);
NixBOOL		nixSourceStreamAppendBuffer(STNix_Engine* engAbs, const NixUI16 sourceIndex, const NixUI16 streamBufferIndex);
NixBOOL		nixSourceEmptyQueue(STNix_Engine* engAbs, const NixUI16 sourceIndex);
NixBOOL		nixSourceHaveBuffer(STNix_Engine* engAbs, const NixUI16 sourceIndex, const NixUI16 bufferIndex);
	
//Audio groups
NixBOOL		nixSrcGroupIsEnabled(STNix_Engine* engAbs, const NixUI8 groupIndex);
NixFLOAT	nixSrcGroupGetVolume(STNix_Engine* engAbs, const NixUI8 groupIndex);
void		nixSrcGroupSetEnabled(STNix_Engine* engAbs, const NixUI8 groupIndex, const NixBOOL enabled);
void		nixSrcGroupSetVolume(STNix_Engine* engAbs, const NixUI8 groupIndex, const NixFLOAT volume);

//Capture
NixBOOL		nixCaptureInit(STNix_Engine* engAbs, const STNix_audioDesc* audioDesc, const NixUI16 buffersCount, const NixUI16 samplesPerBuffer, PTRNIX_CaptureBufferFilledCallback bufferCaptureCallback, void* bufferCaptureCallbackUserData);
void		nixCaptureFinalize(STNix_Engine* engAbs);
NixBOOL		nixCaptureIsOnProgress(STNix_Engine* engAbs);
NixBOOL		nixCaptureStart(STNix_Engine* engAbs);
void		nixCaptureStop(STNix_Engine* engAbs);
NixUI32		nixCaptureFilledBuffersCount(STNix_Engine* engAbs);
NixUI32		nixCaptureFilledBuffersSamples(STNix_Engine* engAbs);
float		nixCaptureFilledBuffersSeconds(STNix_Engine* engAbs);
//Note: when a BufferFilledCallback is defined, the buffers are automatically released after invoking the callback.
void		nixCaptureFilledBuffersRelease(STNix_Engine* engAbs, NixUI32 quantBuffersToRelease);

//Debug
void		nixDbgPrintSourcesStatus(STNix_Engine* engAbs);

#ifdef __cplusplus
}
#endif
		
#endif
