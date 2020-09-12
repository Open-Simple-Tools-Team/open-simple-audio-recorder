//
//  osaudiorecorder.c
//  osaudiorecorder-core-osx
//
//  Created by Marcos Ortega on 9/8/20.
//

//-----------------------------------
//- Open Simple Audio Recorder - Core
//-----------------------------------

#include "osaudiorecorder.h"
#include <stdlib.h>	//malloc
#include <string.h>	//memset
#include "nixtla-audio/nixtla-audio.h"

#ifndef _WIN32
#	include <pthread.h> //for enconding in secondary thread	
#endif

//#define OSAR_ASSERTS_ACTIVATED
//#define OSAR_SILENT_MODE
#define OSAR_VERBOSE_MODE

#if defined(__ANDROID__) //Android
	#include <android/log.h>
	#define OSAR_PRINTF_ALLWAYS(STR_FMT, ...)		__android_log_print(ANDROID_LOG_INFO, "Nix", STR_FMT, ##__VA_ARGS__)
#elif defined(__QNX__) //BB10
	#define OSAR_PRINTF_ALLWAYS(STR_FMT, ...)		fprintf(stdout, "Nix, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
#else
	#define OSAR_PRINTF_ALLWAYS(STR_FMT, ...)		printf("Nix, " STR_FMT, ##__VA_ARGS__)
#endif

#if defined(OSAR_SILENT_MODE)
	#define OSAR_PRINTF_INFO(STR_FMT, ...)			((void)0)
	#define OSAR_PRINTF_ERROR(STR_FMT, ...)			((void)0)
	#define OSAR_PRINTF_WARNING(STR_FMT, ...)		((void)0)
#else
	#if defined(__ANDROID__) //Android
		#include <android/log.h>
		#ifndef OSAR_VERBOSE_MODE
		#define OSAR_PRINTF_INFO(STR_FMT, ...)		((void)0)
		#else
		#define OSAR_PRINTF_INFO(STR_FMT, ...)		__android_log_print(ANDROID_LOG_INFO, "Nix", STR_FMT, ##__VA_ARGS__)
		#endif
		#define OSAR_PRINTF_ERROR(STR_FMT, ...)		__android_log_print(ANDROID_LOG_ERROR, "Nix", "ERROR, " STR_FMT, ##__VA_ARGS__)
		#define OSAR_PRINTF_WARNING(STR_FMT, ...)	__android_log_print(ANDROID_LOG_WARN, "Nix", "WARNING, " STR_FMT, ##__VA_ARGS__)
	#elif defined(__QNX__) //BB10
		#ifndef OSAR_VERBOSE_MODE
		#define OSAR_PRINTF_INFO(STR_FMT, ...)		((void)0)
		#else
		#define OSAR_PRINTF_INFO(STR_FMT, ...)		fprintf(stdout, "Nix, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
		#endif
		#define OSAR_PRINTF_ERROR(STR_FMT, ...)		fprintf(stderr, "Nix ERROR, " STR_FMT, ##__VA_ARGS__); fflush(stderr)
		#define OSAR_PRINTF_WARNING(STR_FMT, ...)	fprintf(stdout, "Nix WARNING, " STR_FMT, ##__VA_ARGS__); fflush(stdout)
	#else
		#ifndef OSAR_VERBOSE_MODE
		#define OSAR_PRINTF_INFO(STR_FMT, ...)		((void)0)
		#else
		#define OSAR_PRINTF_INFO(STR_FMT, ...)		printf("Nix, " STR_FMT, ##__VA_ARGS__)
		#endif
		#define OSAR_PRINTF_ERROR(STR_FMT, ...)		printf("Nix ERROR, " STR_FMT, ##__VA_ARGS__)
		#define OSAR_PRINTF_WARNING(STR_FMT, ...)	printf("Nix WARNING, " STR_FMT, ##__VA_ARGS__)
	#endif
#endif

//--------------------
//- RIFF/WAV(E) format
//--------------------

typedef struct STARRiffWaveChunckHead_ {
	char	cid[4];	//chunk-id
	NixUI32 size;	//size excluding one-bytepadding
} STARRiffWaveChunckHead;

#define WAVE_FORMAT_PCM			1
#define WAVE_FORMAT_IEEE_FLOAT	3

typedef struct STARRiffWaveChunckFmt_ {
	STARRiffWaveChunckHead	head;
	NixUI16					format;		//WAVE_FORMAT_PCM=1 WAVE_FORMAT_IEEE_FLOAT=3
	NixUI16					channels;
	NixUI32					samplesPerSec;
	NixUI32					avgBytesPerSec;
	NixUI16					blocksAlign;
	NixUI16					bitsPerSample;
} STARRiffWaveChunckFmt;

void OSAudioRecorder_playerFillBuffers(STOSAudioRecorder* obj);
void OSAudioRecorder_CaptureBufferFilledCallback(STNix_Engine* engAbs, void* userdata, const STNix_audioDesc audioDesc, const NixUI8* audioData, const NixUI32 audioDataBytes, const NixUI32 audioDataSamples);

//Recording state

typedef struct STARRecordingStats_ {
	STNix_audioDesc desc;
	unsigned long samplesCount;
	unsigned long samplesBytes;
	float samplesSecs;
} STARRecordingStats;

typedef struct STARRecordingState_ {
	STNix_audioDesc		desc;
	STARRecordingStats*	stats;
	FILE*				file;
	long				waveSzPos;
	long				dataHeadPos;
	OSARBool			isPaused;
	//Avgs of samples
	OSARBool			avgsKeep;
	long				avgsSum;
	long				avgsCount;
} STARRecordingState;

//Playing state

typedef struct STARPlayingState_ {
	STNix_audioDesc		desc;
	FILE*				file;
	long				fileStreamPos;	//current data/stream pos
	long				waveDataPos;
	long				waveDataSz;
	NixUI16*			buffsIds;		//
	int					buffsIdsSz;		//
	unsigned char*		buffTmp;		//buffer for loading stream
	int					buffTmpSz;		//size
	NixUI16				iSource;
	OSARBool			isPaused;
} STARPlayingState;

//-----------------
//- Encoder plugins
//-----------------

typedef struct STAREncoderMethods_ {
	OSAudioRecorderEncoderMethod_start	start;
	OSAudioRecorderEncoderMethod_feed	feed;
	OSAudioRecorderEncoderMethod_end	end;
} STAREncoderMethods;

typedef struct STAREncoderPlugin_ {
	char*				formatId;
	STAREncoderMethods	methods;
} STAREncoderPlugin;

//Opus
void* OSAudioRecorderEncoderOpusStart(FILE* dst, int channels, int bitsPerSample, int samplerate, int blockAlign, int samplesTotal);
OSARBool OSAudioRecorderEncoderOpusFeed(void* userData, FILE* dst, void* samples, long samplesBytes, long samplesCount);
OSARBool OSAudioRecorderEncoderOpusEnd(void* userData, FILE* dst);

//Flac
void* OSAudioRecorderEncoderFlacStart(FILE* dst, int channels, int bitsPerSample, int samplerate, int blockAlign, int samplesTotal);
OSARBool OSAudioRecorderEncoderFlacFeed(void* userData, FILE* dst, void* samples, long samplesBytes, long samplesCount);
OSARBool OSAudioRecorderEncoderFlacEnd(void* userData, FILE* dst);

//----------
//- Encoding
//----------

typedef struct STNBThread_ {
#	ifdef _WIN32
	HANDLE				handle;
	DWORD				threadId;
#	else
	pthread_t			thread;
#	endif
} STNBThread;

#ifdef _WIN32
typedef DWORD WINAPI (*NBThreadRoutinePtr)(LPVOID param);
#else
typedef void* (*NBThreadRoutinePtr)(void* param);
#endif

void		NBThread_init(STNBThread* obj);
OSARBool	NBThread_start(STNBThread* obj, NBThreadRoutinePtr routine, void* param);
void		NBThread_release(STNBThread* obj);
	
typedef struct STAREncodingState_ {
	OSARBool			isRunning;
	OSARBool			stopSignal;
	FILE*				fileIn;
	FILE*				fileOut;
	STAREncoderMethods	methods;
	void*				methodsParam;
	STNix_audioDesc		audDesc;
	unsigned long		samplesExpectedCount;
	unsigned long		samplesProcessedCount;
	STNBThread*			thread;
} STAREncodingState;

//--------
//- Engine
//--------

typedef struct STOSAudioRecorderOpq_ {
	STNix_Engine		nix;			//audio-API abstraction layer
	STAREncoderPlugin*	encoders;		//Encoders
	int					encodersSz;		//Encoders
	STARRecordingStats	lastStats;		//last (and current) recording stats
	STARRecordingState* curRecording;	//current recording state
	STARPlayingState*	curPlaying;		//current playing state
	STAREncodingState*	curEncoding;	//current enconding state
} STOSAudioRecorderOpq;


OSARBool OSAudioRecorder_init(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	STNix_Engine nix;
	memset(obj, 0, sizeof(*obj));
	if(!nixInit(&nix, 0)){
		OSAR_PRINTF_ERROR("'nixInit' failed.\n");
	} else {
		STOSAudioRecorderOpq* opq = obj->opaque = (STOSAudioRecorderOpq*)malloc(sizeof(STOSAudioRecorderOpq));
		memset(opq, 0, sizeof(*opq));
		opq->nix = nix;
		//Opus encoder
		OSAudioRecorder_encoderAdd(obj, "opus", OSAudioRecorderEncoderOpusStart, OSAudioRecorderEncoderOpusFeed, OSAudioRecorderEncoderOpusEnd);
		//Flac encoder
		OSAudioRecorder_encoderAdd(obj, "flac", OSAudioRecorderEncoderFlacStart, OSAudioRecorderEncoderFlacFeed, OSAudioRecorderEncoderFlacEnd);
		//
		r = TRUE;
	}
	return r;
} 

void OSAudioRecorder_release(STOSAudioRecorder* obj){
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		nixFinalize(&opq->nix);
		//Encoders
		if(opq->encoders != NULL){
			int i; for(i = 0; i < opq->encodersSz; i++){
				STAREncoderPlugin* enc = &opq->encoders[i];
				if(enc->formatId != NULL){
					free(enc->formatId);
					enc->formatId = NULL;
				}
			}
			free(opq->encoders);
			opq->encoders = NULL;
			opq->encodersSz = 0;
		}
		//Free
		free(obj->opaque);
		obj->opaque = NULL;
	}
}

//

void OSAudioRecorder_tick(STOSAudioRecorder* obj){
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		nixTick(&opq->nix);
		//
		if(opq->curPlaying != NULL){
			OSAudioRecorder_playerFillBuffers(obj);
		}
	}
}

//----------------
//- Recorder (WAV)
//----------------

OSARBool OSAudioRecorder_start(STOSAudioRecorder* obj, const char* wavFilepath, const int buffersPerSec, const OSARBool keepAvgsOfSamples){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording == NULL && wavFilepath != NULL && buffersPerSec > 0){
			NixUI32 capabilities;
			nixContextActivate(&opq->nix); //good practice (does nothing for OpenSL/Android)
			//Eval capabilities
			capabilities = nixCapabilities(&opq->nix);
			if((capabilities & NIX_CAP_AUDIO_CAPTURE) == 0){
				OSAR_PRINTF_ERROR("current device cannot capture audio; capabilities(%d).\n", capabilities);
			} else {
				//https://developer.android.com/reference/android/media/AudioRecord
				//44100Hz is currently the only rate that is guaranteed
				//to work on all devices, but other rates such as 22050,
				//16000, and 11025 may work on some devices.
				FILE* file = NULL;
				const char* filepath = wavFilepath;
				long waveSzPos = 0, dataHeadPos = 0;
				STNix_audioDesc audDesc;
				memset(&audDesc, 0, sizeof(audDesc));
				audDesc.samplesFormat	= ENNix_sampleFormat_int;
				audDesc.channels		= 1;
				audDesc.bitsPerSample	= 16;
				audDesc.samplerate		= 16000; //8000 (telephone/walkie-talkie), 16000 (VoIP), 22050 (MPEG audio), 44100 (Audio CD) 
				audDesc.blockAlign		= (audDesc.bitsPerSample / 8) * audDesc.channels;
				//Open file and write header
				{
					file = fopen(filepath, "wb+");
					if(file == NULL){
						OSAR_PRINTF_ERROR("'fopen' failed for path '%s'.\n", filepath);
					} else {
						//Write WAV/RIFF header
						//RIFF
						{
							NixUI32 fileSz = 0;
							fwrite("RIFF", 4, 1, file);
							waveSzPos	= ftell(file);
							fwrite(&fileSz, sizeof(fileSz), 1, file);
						}
						//Wave
						{
							fwrite("WAVE", 4, 1, file);
						}
						//format chunk
						{
							STARRiffWaveChunckFmt fmtHead;
							memset(&fmtHead, 0, sizeof(fmtHead));
							fmtHead.head.cid[0]		= 'f';
							fmtHead.head.cid[1]		= 'm';
							fmtHead.head.cid[2]		= 't';
							fmtHead.head.cid[3]		= ' ';
							fmtHead.head.size		= sizeof(fmtHead) - sizeof(fmtHead.head);
							fmtHead.format			= (audDesc.samplesFormat == ENNix_sampleFormat_float ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM);
							fmtHead.channels		= audDesc.channels;
							fmtHead.samplesPerSec	= audDesc.samplerate;
							fmtHead.avgBytesPerSec	= (audDesc.samplerate * audDesc.bitsPerSample * audDesc.channels) / 8;
							fmtHead.blocksAlign		= audDesc.blockAlign;
							fmtHead.bitsPerSample	= audDesc.bitsPerSample;
							fwrite(&fmtHead, sizeof(fmtHead), 1, file);
						}
						//data chunck
						{
							STARRiffWaveChunckHead head;
							memset(&head, 0, sizeof(head));
							head.cid[0]	= 'd';
							head.cid[1]	= 'a';
							head.cid[2]	= 't';
							head.cid[3]	= 'a';
							head.size	= 0;
							dataHeadPos	= ftell(file);
							fwrite(&head, sizeof(head), 1, file);
						}
						fflush(file);
					}
				}
				//Start recording
				if(file != NULL){
					const NixUI16 bufferedSecs = 1;
					const NixUI16 buffersCount = bufferedSecs * buffersPerSec;
					const NixUI16 samplesPerBuffer = (audDesc.samplerate / buffersPerSec);
					STARRecordingState* state = (STARRecordingState*)malloc(sizeof(STARRecordingState));
					memset(state, 0, sizeof(*state));
					memset(&opq->lastStats, 0, sizeof(opq->lastStats));
					opq->lastStats.desc	= audDesc;
					state->desc			= audDesc;
					state->stats		= &opq->lastStats;
					state->file			= file;
					state->waveSzPos	= waveSzPos;
					state->dataHeadPos	= dataHeadPos;
					state->isPaused		= FALSE;
					//
					state->avgsKeep		= keepAvgsOfSamples;
					state->avgsSum		= 0;
					state->avgsCount	= 0;
					//
					opq->curRecording	= state;
					if(!nixCaptureInit(&opq->nix, &audDesc, buffersCount, samplesPerBuffer, &OSAudioRecorder_CaptureBufferFilledCallback, &opq->curRecording)){
						OSAR_PRINTF_ERROR("could not start audio capture.\n");
					} else if(!nixCaptureStart(&opq->nix)){
						OSAR_PRINTF_ERROR("could not start audio capture.\n");
					} else {
						//consume
						state		= NULL;
						file		= NULL;
						r			= TRUE;
					}
					//Release (if not consumed)
					if(state != NULL){
						opq->curRecording = NULL;
						free(state);
						state = NULL;
					}
				}
				//Release (if not consumed)
				if(file != NULL){
					fclose(file);
					file = NULL;
				}
			}
		}
	}
	return r;
}

void OSAudioRecorder_CaptureBufferFilledCallback(STNix_Engine* engAbs, void* userdata, const STNix_audioDesc audioDesc, const NixUI8* audioData, const NixUI32 audioDataBytes, const NixUI32 audioDataSamples){
	//Write wave data
	STARRecordingState** stateRef = (STARRecordingState**)userdata;
	if(stateRef != NULL && audioData != NULL && audioDataBytes > 0){
		STARRecordingState* state = *stateRef;
		if(state != NULL){
			if(state->file != NULL && !state->isPaused){
				fwrite(audioData, audioDataBytes, 1, state->file);
				fflush(state->file);
				//Stats
				if(state->stats != NULL){
					state->stats->samplesCount += audioDataSamples;
					state->stats->samplesBytes += audioDataBytes;
					if(state->stats->desc.samplerate > 0){
						state->stats->samplesSecs = (float)(state->stats->samplesCount / 1000) / ((float)state->stats->desc.samplerate / 1000.f);
						//state->stats->samplesSecs = ((float)state->stats->samplesCount / (float)state->stats->desc.samplerate);
						//OSAR_PRINTF_ERROR("samplesCount(%lu) samplesBytes(%lu) samplesSecs(%f).\n", state->stats->samplesCount, state->stats->samplesBytes, state->stats->samplesSecs);
					}
				}
				//
				if(state->avgsKeep){
					//ToDo: currently asuming 16-bits, 1-channel, no-padding
					//implement explicit in the future.
					NixSI16 sample;
					NixUI8* sampleBytes			= (NixUI8*)&sample;
					const NixUI8* data			= (const NixUI8*)audioData;
					const NixUI8* dataAfterEnd	= data + (audioDataBytes / sizeof(*data));
					while(data < dataAfterEnd){
						//Big-endien to little-endien
						sampleBytes[0] = data[1];
						sampleBytes[1] = data[0];
						//
						state->avgsSum += sample;
						state->avgsCount++;
						//
						data += 2;
					}
				}
			}
		}
	}
}

OSARBool OSAudioRecorder_stop(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording != NULL){
			STARRecordingState* state = opq->curRecording;
			//End
			nixCaptureStop(&opq->nix);
			//update the size fo the 'data' sub-chunk
			{
				long finalPos = ftell(state->file);
				if(state->dataHeadPos < finalPos){
					const NixUI32 dataSz = (NixUI32)(finalPos - state->dataHeadPos - sizeof(STARRiffWaveChunckHead));
					//Write 'data' padding byte
					if((dataSz % 2) != 0){
						const char padd = '\0';
						fwrite(&padd, sizeof(padd), 1, state->file);
						finalPos++;
					}
					fseek(state->file, state->dataHeadPos + 4, SEEK_SET);
					fwrite(&dataSz, sizeof(dataSz), 1, state->file);
					fflush(state->file);
					//Update the size of the 'WAVE' chunk
					if(finalPos > 8){
						NixUI32 fileSz = (NixUI32)(finalPos - 8);
						fseek(state->file, state->waveSzPos, SEEK_SET);
						fwrite(&fileSz, sizeof(fileSz), 1, state->file);
						fflush(state->file);
					}
				}
			}
			fclose(state->file);
			//Free
			free(opq->curRecording);
			opq->curRecording = NULL;
			//Finalize
			nixCaptureFinalize(&opq->nix);
			r = TRUE;
		}
	}
	return r;
}

OSARBool OSAudioRecorder_reset(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording == NULL){
			memset(&opq->lastStats, 0, sizeof(opq->lastStats));
			r = TRUE;
		}
	}
	return r;
}

OSARBool OSAudioRecorder_isActive(STOSAudioRecorder* obj){ //recording or paused
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording != NULL){
			r = TRUE;
		}
	}
	return r;
}

OSARBool OSAudioRecorder_isRecording(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording != NULL){
			if(!opq->curRecording->isPaused){
				r = TRUE;
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorder_isPaused(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording != NULL){
			if(opq->curRecording->isPaused){
				r = TRUE;
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorder_pause(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording != NULL){
			opq->curRecording->isPaused = TRUE;
			r = TRUE;
		}
	}
	return r;
}

OSARBool OSAudioRecorder_resume(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording != NULL){
			opq->curRecording->isPaused = FALSE;
			r = TRUE;
		}
	}
	return r;
}

//current or last recording stats

unsigned long OSAudioRecorder_samplesCount(STOSAudioRecorder* obj){
	unsigned long r = 0;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		r = opq->lastStats.samplesCount;
	}
	return r;
}

unsigned long OSAudioRecorder_samplesBytes(STOSAudioRecorder* obj){
	unsigned long r = 0;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		r = opq->lastStats.samplesBytes;
	}
	return r;
}

float OSAudioRecorder_samplesSecs(STOSAudioRecorder* obj){
	float r = 0;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		r = opq->lastStats.samplesSecs;
	}
	return r;
}

//Avg of samples from -1 to 1 (warning: overflows if not manually reset preiodically)
float OSAudioRecorder_samplesRelAvg(STOSAudioRecorder* obj, const OSARBool resetAvg){
	float r = 0.0f;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curRecording != NULL){
			if(opq->curRecording->avgsCount > 0){
				//ToDo: currently asuming 16-bits, 1-channel, signed
				//implement explicit in the future.
				r = ((float)(opq->curRecording->avgsSum / opq->curRecording->avgsCount) / 32767.f);
				if(resetAvg){
					opq->curRecording->avgsSum		= 0;
					opq->curRecording->avgsCount	= 0;
				}
			}
		}
	}
	return r;
}

//--------------
//- Player (WAV)
//--------------

OSARBool OSAudioRecorder_playerLoad(STOSAudioRecorder* obj, const char* filepath, const int buffersPerSec){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying == NULL && filepath != NULL && buffersPerSec > 0){
			nixContextActivate(&opq->nix); //good practice (does nothing for OpenSL/Android)
			FILE* file = NULL;
			STNix_audioDesc audDesc;
			memset(&audDesc, 0, sizeof(audDesc));
			//Open file and read header
			file = fopen(filepath, "rb");
			if(file == NULL){
				OSAR_PRINTF_ERROR("'fopen' failed for path '%s'.\n", filepath);
			} else {
				r = TRUE;
				//Read WAV/RIFF header
				//RIFF
				if(r){
					char headRIFF[4];
					NixUI32 fileSz = 0;
					if(fread(headRIFF, sizeof(headRIFF), 1, file) != 1){
						OSAR_PRINTF_ERROR("'fread' failed for RIFF header ('%s').\n", filepath);
						r = FALSE;
					} else if(headRIFF[0] != 'R' || headRIFF[1] != 'I' || headRIFF[2] != 'F' || headRIFF[3] != 'F'){
						OSAR_PRINTF_ERROR("file is not RIFF ('%s').\n", filepath);
						r = FALSE;
					} else if(fread(&fileSz, sizeof(fileSz), 1, file) != 1){
						OSAR_PRINTF_ERROR("'fread' failed for RIFF fileSz ('%s').\n", filepath);
						r = FALSE;
					}
				}
				//Wave
				if(r){
					char headWAVE[4];
					if(fread(headWAVE, sizeof(headWAVE), 1, file) != 1){
						OSAR_PRINTF_ERROR("'fread' failed for WAVE header ('%s').\n", filepath);
						r = FALSE;
					} else if(headWAVE[0] != 'W' || headWAVE[1] != 'A' || headWAVE[2] != 'V' || headWAVE[3] != 'E'){
						OSAR_PRINTF_ERROR("file is not WAVE ('%s').\n", filepath);
						r = FALSE;
					}
				}
				//format chunk
				if(r){
					STARRiffWaveChunckFmt fmtHead;
					if(fread(&fmtHead, sizeof(fmtHead), 1, file) != 1){
						OSAR_PRINTF_ERROR("'fread' failed for fmtHead ('%s').\n", filepath);
						r = FALSE;
					} else if(fmtHead.head.cid[0] != 'f' || fmtHead.head.cid[1] != 'm' || fmtHead.head.cid[2] != 't' || fmtHead.head.cid[3] != ' '){
						OSAR_PRINTF_ERROR("chunck is not 'fmt ' ('%s').\n", filepath);
						r = FALSE;
					} else if(fmtHead.format != WAVE_FORMAT_PCM){
						OSAR_PRINTF_ERROR("WAVE data is not PCM ('%s').\n", filepath);
						r = FALSE;
					} else if(fmtHead.channels != 1){
						OSAR_PRINTF_ERROR("WAVE data is not 1-channel ('%s').\n", filepath);
						r = FALSE;
					} else if(fmtHead.bitsPerSample != 16){
						OSAR_PRINTF_ERROR("WAVE data is not 16-bits ('%s').\n", filepath);
						r = FALSE;
					} else {
						audDesc.samplesFormat	= ENNix_sampleFormat_int;
						audDesc.channels		= fmtHead.channels;
						audDesc.bitsPerSample	= fmtHead.bitsPerSample;
						audDesc.samplerate		= fmtHead.samplesPerSec;
						audDesc.blockAlign		= fmtHead.blocksAlign;
					}
				}
				//data chunck
				if(r){
					STARRiffWaveChunckHead head;
					if(fread(&head, sizeof(head), 1, file) != 1){
						OSAR_PRINTF_ERROR("'fread' failed for dataHead ('%s').\n", filepath);
						r = FALSE;
					} else if(head.cid[0] != 'd' || head.cid[1] != 'a' || head.cid[2] != 't' || head.cid[3] != 'a'){
						OSAR_PRINTF_ERROR("chunck is not 'data' ('%s').\n", filepath);
						r = FALSE;
					} else {
						const float buffersSecs		= 0.25f;
						NixUI16 buffersCount		= buffersPerSec * buffersSecs;
						if(buffersCount <= 0){
							buffersCount = 1;
						}
						STARPlayingState* state		= (STARPlayingState*)malloc(sizeof(STARPlayingState));
						const NixUI16 iSourceWav	= nixSourceAssignStream(&opq->nix, NIX_TRUE, 0, NULL, NULL, buffersCount, NULL, NULL);
						if(iSourceWav == 0){
							OSAR_PRINTF_ERROR("nixSourceAssignStream failed for ('%s').\n", filepath);
							r = FALSE;
						} else {
							NixUI16* buffs		= (NixUI16*)malloc(sizeof(NixUI16) * buffersCount);
							nixSourcePause(&opq->nix, iSourceWav);
							nixSourceSetRepeat(&opq->nix, iSourceWav, FALSE);
							nixSourceSetVolume(&opq->nix, iSourceWav, 1.0f);
							memset(state, 0, sizeof(*state));
							state->desc			= audDesc;
							state->file			= file;
							state->waveDataPos	= ftell(file);
							state->fileStreamPos = state->waveDataPos; 
							state->waveDataSz	= head.size;
							state->iSource		= iSourceWav;
							state->isPaused		= TRUE;
							state->buffsIdsSz	= buffersCount;
							state->buffsIds		= buffs;
							{
								int i; for(i = 0; i < buffersCount; i++){
									buffs[i] = nixBufferWithData(&opq->nix, &audDesc, NULL, 0);
									if(buffs[i] <= 0){
										OSAR_PRINTF_ERROR("nixBufferWithData failed for ('%s').\n", filepath);
										r = FALSE;
										break;
									}
								}
							}
							if(r){
								state->buffTmpSz	= (audDesc.samplerate * audDesc.blockAlign / buffersPerSec);
								state->buffTmp		= (unsigned char*)malloc(state->buffTmpSz);
								opq->curPlaying		= state;
								//consume
								file = NULL;
								state = NULL;
								buffs = NULL;
							}
							//Release (if not consumed)
							if(buffs != NULL){
								int i; for(i = 0; i < buffersCount; i++){
									nixBufferRelease(&opq->nix, buffs[i]);
								}
								free(buffs);
								buffs = NULL;
							}
						}
						//Release (if not consumed)
						if(state != NULL){
							free(state);
							state = NULL;
						}
					}
				}
				//Close (if not consnumed)
				if(file != NULL){
					fclose(file);
					file = NULL;
				}
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorder_playerStop(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying; 
			//
			nixSourceStop(&opq->nix, state->iSource);
			nixSourceRelease(&opq->nix, state->iSource);
			//
			if(state->buffsIds != NULL && state->buffsIdsSz > 0){
				int i; for(i = 0; i < state->buffsIdsSz; i++){
					nixBufferRelease(&opq->nix, state->buffsIds[i]);
				}
				free(state->buffsIds);
				state->buffsIds = NULL;
				state->buffsIdsSz = 0;
			}
			if(state->buffTmp != NULL){
				free(state->buffTmp);
				state->buffTmp = NULL;
				state->buffTmpSz = 0;
			}
			//
			fclose(state->file);
			state->file = NULL;
			//
			free(state);
			state = opq->curPlaying = NULL;
			//
			r = TRUE;
		}
	}
	return r;
}

void OSAudioRecorder_playerFillBuffers(STOSAudioRecorder* obj){
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying;
			if(!state->isPaused){
				OSARBool errFound = FALSE;
				size_t samplesCount = 0;
				while(nixSourceGetBuffersCount(&opq->nix, state->iSource) < state->buffsIdsSz && !errFound){
					fseek(state->file, state->fileStreamPos, SEEK_SET);
					samplesCount = fread(state->buffTmp, state->desc.blockAlign, (state->buffTmpSz / state->desc.blockAlign), state->file);
					if(samplesCount <= 0){
						//end-of-stream
						break;
					} else {
						//find a buffer (at least one should be available)
						NixUI16 buddId = 0;
						int i; for(i = 0; i < state->buffsIdsSz; i++){
							if(!nixSourceHaveBuffer(&opq->nix, state->iSource, state->buffsIds[i])){
								if(!nixBufferSetData(&opq->nix, state->buffsIds[i], &state->desc, state->buffTmp, (NixUI32)(samplesCount * state->desc.blockAlign))){
									errFound = TRUE;
								} else if(!nixSourceStreamAppendBuffer(&opq->nix, state->iSource, state->buffsIds[i])){
									errFound = TRUE;
								} else {
									buddId = state->buffsIds[i];
								}
								break;
							}
						}
						if(buddId == 0){
							//Something went wrong
							break;
						}
						state->fileStreamPos += (samplesCount * state->desc.blockAlign);
					}
				}
				//In case the stream paused due to lack of buffers.
				if(!nixSourceIsPlaying(&opq->nix, state->iSource)){
					nixSourcePlay(&opq->nix, state->iSource);
				}
			}
		}
	}
}

OSARBool OSAudioRecorder_playerPlay(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying;
			if(state->isPaused){
				nixSourcePlay(&opq->nix, state->iSource);
				OSAudioRecorder_playerFillBuffers(obj);
				state->isPaused = FALSE;
				r = TRUE;
			}
		}
	}
	return r;
} 

OSARBool OSAudioRecorder_playerPause(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying; 
			if(!state->isPaused){
				nixSourcePause(&opq->nix, state->iSource);
				state->isPaused = TRUE;
				r = TRUE;
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorder_playerIsLoaded(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			r = TRUE;
		}
	}
	return r;
}

OSARBool OSAudioRecorder_playerIsPaused(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying; 
			if(state->isPaused){
				r = TRUE;
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorder_playerSeekRelPos(STOSAudioRecorder* obj, const float relPos){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying; 
			if(state->waveDataSz > 0 && state->desc.samplerate > 8 && state->desc.blockAlign > 0){
				unsigned long scale = 100000; 
				unsigned long relPosFixed = relPos * (float)scale;
				state->fileStreamPos = state->waveDataPos + (((state->waveDataSz * relPosFixed / scale) / state->desc.blockAlign) * state->desc.blockAlign);
				r = TRUE;
			}
		}
	}
	return r;
}

float OSAudioRecorder_playerRelProgress(STOSAudioRecorder* obj){
	float r = 0.0f;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying; 
			if(state->waveDataSz > 0 && state->desc.samplerate > 8){
				const long divider = (state->desc.samplerate / 8);
				r = (float)((state->fileStreamPos - state->waveDataPos) / divider) / (float)(state->waveDataSz / divider);
			}
		}
	}
	return r;
}

float OSAudioRecorder_playerSecsTotal(STOSAudioRecorder* obj){
	float r = 0.0f;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curPlaying != NULL){
			STARPlayingState* state = opq->curPlaying;
			if(state->waveDataSz > 0 && state->desc.samplerate > 8 && state->desc.blockAlign > 0){
				const long samplesTotal = (state->waveDataSz / state->desc.blockAlign);
				const long divider = (state->desc.samplerate / 8);
				r = (float)(samplesTotal / divider) / (float)(state->desc.samplerate / divider);
			}
		}
	}
	return r;
}

//------------
//- Encoder (from WAV to ...)
//------------

void NBThread_init(STNBThread* obj){
#	ifdef _WIN32
	obj->handle		= NULL;
	obj->threadId	= 0;
#	else
	//obj->thread
#	endif
}

OSARBool NBThread_start(STNBThread* obj, NBThreadRoutinePtr routine, void* param){
	OSARBool r = FALSE;
#	ifdef _WIN32
	//NBASSERT(FALSE) //ToDo: implement stackSz
	/*if(opq->handle == NULL && opq->threadId == 0){
		DWORD		threadId = 0;
		opq->created	= TRUE;
		opq->isRunning	= TRUE;
		opq->routine	= routine;
		opq->param		= param;
		opq->retVal		= 0;
		if((opq->handle = CreateThread(NULL,					//LPSECURITY_ATTRIBUTES secAttr,
									   opq->stackSz,			//SIZE_T stackSize,
									   NBThread_runRoutine,	//LPTHREAD_START_ROUTINE threadFunc,
									   obj,					//LPVOID param,
									   (DWORD)0,				//DWORD flags,
									   &opq->threadId			//LPDWORD threadID
									   )) == NULL){
			opq->created	= FALSE;
			opq->isRunning	= FALSE;
			NBASSERT(opq->handle == NULL)
			opq->threadId	= 0;
			opq->routine	= NULL;
			opq->param		= NULL;
			opq->retVal		= 0;
		} else {
			r = TRUE;
		}
	}*/
#	else
	OSARBool cfgrd = TRUE;
	pthread_attr_t attrs;
	pthread_attr_init(&attrs);
	//Set joinabled or detached (determine when system's resources will be released)
	if(cfgrd){
		if(pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED) != 0){
			cfgrd = FALSE;
		}
	}
	if(cfgrd){
		int rp;
		if((rp = pthread_create(&obj->thread, &attrs, routine, param)) == 0){
			r = TRUE;
		}
	}
	pthread_attr_destroy(&attrs);
#	endif
	return r;
}

void NBThread_release(STNBThread* obj){
#	ifdef _WIN32
	if(opq->handle != NULL){
		CloseHandle(opq->handle);
		opq->handle		= NULL;
		opq->threadId	= 0;
	}
#	else
	//
#	endif
}

OSARBool OSAudioRecorder_encoderAdd(STOSAudioRecorder* obj, const char* formatId, OSAudioRecorderEncoderMethod_start methodStart, OSAudioRecorderEncoderMethod_feed methodFeed, OSAudioRecorderEncoderMethod_end methodEnd){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		//Search
		int i; for(i = 0; i < opq->encodersSz; i++){
			const STAREncoderPlugin* enc = &opq->encoders[i];
			if(strcmp(enc->formatId, formatId) == 0){
				break;
			}
		}
		if(i == opq->encodersSz){
			STAREncoderPlugin* encsArr = (STAREncoderPlugin*)malloc(sizeof(STAREncoderPlugin) * (opq->encodersSz + 1));
			//Add current
			if(opq->encoders != NULL){
				if(opq->encodersSz > 0){
					memcpy(encsArr, opq->encoders, sizeof(STAREncoderPlugin) * opq->encodersSz);
				}
				free(opq->encoders);
			}
			//Add new
			{
				const unsigned long formatIdLen = strlen(formatId);
				STAREncoderPlugin* enc = &encsArr[opq->encodersSz];
				memset(enc, 0, sizeof(*enc));
				enc->formatId		= (char*)malloc(formatIdLen + 1);
				memcpy(enc->formatId, formatId, formatIdLen);
				enc->formatId[formatIdLen] = '\0';
				enc->methods.start	= methodStart;
				enc->methods.feed	= methodFeed;
				enc->methods.end	= methodEnd;
			}
			opq->encoders = encsArr;
			opq->encodersSz++;
			r = TRUE;
		}
	}
	//Add
	return r;
}

#ifdef _WIN32
DWORD WINAPI OSAudioRecorder_encoderMethod_(LPVOID param)
#else
void* OSAudioRecorder_encoderMethod_(void* param)
#endif
{
	STAREncodingState* state = (STAREncodingState*)param;
	//
	state->isRunning = TRUE;
	//
	{
		const long buffSzSamples = ((long)state->audDesc.samplerate * 60L) / 1000L; //60ms at the time (OPUS frame sizes from 2.5 ms to 60 ms)
		const long buffSz		=  buffSzSamples * state->audDesc.blockAlign;
		unsigned char* buff		= (unsigned char*)malloc(buffSz);
		long samplesRemain, samplesToLoad, samplesLoaded;
		//Feed
		while(!state->stopSignal && state->samplesProcessedCount < state->samplesExpectedCount){
			samplesRemain	= state->samplesExpectedCount - state->samplesProcessedCount;
			samplesToLoad	= (samplesRemain < buffSzSamples ? samplesRemain : buffSzSamples);
			samplesLoaded	= fread(buff, state->audDesc.blockAlign, samplesToLoad, state->fileIn);
			//OSAR_PRINTF_ERROR("samplesLoaded(%ld).\n", samplesLoaded);
			if(samplesLoaded <= 0){
				break;
			} else if(!(state->methods.feed)(state->methodsParam, state->fileOut, buff, (samplesLoaded * state->audDesc.blockAlign), samplesLoaded)){
				break;
			} else {
				state->samplesProcessedCount += samplesLoaded;
			}
		}
		//End
		if(!(state->methods.end)(state->methodsParam, state->fileOut)){
			//
		}
		//Release
		if(buff != NULL){
			free(buff);
			buff = NULL;
		}
		state->methodsParam = NULL;
		if(state->fileOut != NULL){
			fclose(state->fileOut);
			state->fileOut = NULL;
		}
		if(state->fileIn != NULL){
			fclose(state->fileIn);
			state->fileIn = NULL;
		}
	}
	//Release thread
	if(state->thread != NULL){
		NBThread_release(state->thread);
		free(state->thread);
		state->thread = NULL;
	}
	//
	state->isRunning = FALSE;
	//
	return 0;
}

OSARBool OSAudioRecorder_encoderStart(STOSAudioRecorder* obj, const char* filepathSrc, const char* filepathDst, const char* outputFormatId){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curEncoding == NULL){
			int iEnc;
			STAREncoderMethods methods;
			memset(&methods, 0, sizeof(methods));
			//Search encoder
			for(iEnc = 0; iEnc < opq->encodersSz; iEnc++){
				const STAREncoderPlugin* enc = &opq->encoders[iEnc];
				if(strcmp(enc->formatId, outputFormatId) == 0){
					methods = enc->methods;
					break;
				}
			}
			if(iEnc < opq->encodersSz){
				FILE* file = NULL;
				STNix_audioDesc audDesc;
				memset(&audDesc, 0, sizeof(audDesc));
				//Open file and read header
				file = fopen(filepathSrc, "rb");
				if(file == NULL){
					OSAR_PRINTF_ERROR("'fopen' failed for path '%s'.\n", filepathSrc);
				} else {
					r = TRUE;
					//Read WAV/RIFF header
					//RIFF
					if(r){
						char headRIFF[4];
						NixUI32 fileSz = 0;
						if(fread(headRIFF, sizeof(headRIFF), 1, file) != 1){
							OSAR_PRINTF_ERROR("'fread' failed for RIFF header ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(headRIFF[0] != 'R' || headRIFF[1] != 'I' || headRIFF[2] != 'F' || headRIFF[3] != 'F'){
							OSAR_PRINTF_ERROR("file is not RIFF ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(fread(&fileSz, sizeof(fileSz), 1, file) != 1){
							OSAR_PRINTF_ERROR("'fread' failed for RIFF fileSz ('%s').\n", filepathSrc);
							r = FALSE;
						}
					}
					//Wave
					if(r){
						char headWAVE[4];
						if(fread(headWAVE, sizeof(headWAVE), 1, file) != 1){
							OSAR_PRINTF_ERROR("'fread' failed for WAVE header ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(headWAVE[0] != 'W' || headWAVE[1] != 'A' || headWAVE[2] != 'V' || headWAVE[3] != 'E'){
							OSAR_PRINTF_ERROR("file is not WAVE ('%s').\n", filepathSrc);
							r = FALSE;
						}
					}
					//format chunk
					if(r){
						STARRiffWaveChunckFmt fmtHead;
						if(fread(&fmtHead, sizeof(fmtHead), 1, file) != 1){
							OSAR_PRINTF_ERROR("'fread' failed for fmtHead ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(fmtHead.head.cid[0] != 'f' || fmtHead.head.cid[1] != 'm' || fmtHead.head.cid[2] != 't' || fmtHead.head.cid[3] != ' '){
							OSAR_PRINTF_ERROR("chunck is not 'fmt ' ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(fmtHead.format != WAVE_FORMAT_PCM){
							OSAR_PRINTF_ERROR("WAVE data is not PCM ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(fmtHead.channels != 1){
							OSAR_PRINTF_ERROR("WAVE data is not 1-channel ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(fmtHead.bitsPerSample != 16){
							OSAR_PRINTF_ERROR("WAVE data is not 16-bits ('%s').\n", filepathSrc);
							r = FALSE;
						} else {
							audDesc.samplesFormat	= ENNix_sampleFormat_int;
							audDesc.channels		= fmtHead.channels;
							audDesc.bitsPerSample	= fmtHead.bitsPerSample;
							audDesc.samplerate		= fmtHead.samplesPerSec;
							audDesc.blockAlign		= fmtHead.blocksAlign;
						}
					}
					//data chunck
					if(r){
						STARRiffWaveChunckHead head;
						if(fread(&head, sizeof(head), 1, file) != 1){
							OSAR_PRINTF_ERROR("'fread' failed for dataHead ('%s').\n", filepathSrc);
							r = FALSE;
						} else if(head.cid[0] != 'd' || head.cid[1] != 'a' || head.cid[2] != 't' || head.cid[3] != 'a'){
							OSAR_PRINTF_ERROR("chunck is not 'data' ('%s').\n", filepathSrc);
							r = FALSE;
						} else {
							r = FALSE;
							{
								FILE* fileOut = fopen(filepathDst, "wb+");
								if(fileOut == NULL){
									OSAR_PRINTF_ERROR("'fopen' failed for path '%s'.\n", filepathDst);
								} else {
									void* methodsParam = (methods.start)(fileOut, audDesc.channels, audDesc.bitsPerSample, audDesc.samplerate, audDesc.blockAlign, (head.size / audDesc.blockAlign));
									if(methodsParam == NULL){
										OSAR_PRINTF_ERROR("plugin 'start' failed for path '%s'.\n", filepathDst);
									} else {
										STAREncodingState* state = (STAREncodingState*)malloc(sizeof(STAREncodingState));
										memset(state, 0, sizeof(*state));
										state->fileIn		= file;
										state->fileOut		= fileOut;
										state->isRunning	= TRUE;
										state->stopSignal	= FALSE;
										state->methods		= methods;
										state->methodsParam	= methodsParam;
										state->audDesc		= audDesc;
										state->samplesProcessedCount	= 0;
										state->samplesExpectedCount		= (head.size / audDesc.blockAlign);
										//Start threat
										{
											STNBThread* thread = state->thread = (STNBThread*)malloc(sizeof(STNBThread));
											NBThread_init(thread);
											if(!NBThread_start(thread, OSAudioRecorder_encoderMethod_, state)){
												OSAR_PRINTF_ERROR("plugin 'NBThread_start' failed for path '%s'.\n", filepathDst);
											} else {
												//Consume
												opq->curEncoding = state;
												state	= NULL;
												thread	= NULL;
												fileOut	= NULL;
												file	= NULL;
												r		= TRUE;
											}
											//Release (if not consumed)
											if(thread != NULL){
												NBThread_release(thread);
												free(thread);
												thread = NULL;
											}
										}
										//Release (if not consumed)
										if(state != NULL){
											free(state);
											state = NULL;
										}
									}
									//Close (if not consnumed)
									if(fileOut != NULL){
										fclose(fileOut);
										fileOut = NULL;
									}
								}
							}
						}
					}
					//Close (if not consnumed)
					if(file != NULL){
						fclose(file);
						file = NULL;
					}
				}
			}
		}
	}
	//Add
	return r;
}

OSARBool OSAudioRecorder_encoderIsLoaded(STOSAudioRecorder* obj){
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curEncoding != NULL){
			r = TRUE;
		}
	}
	return r;
}

//Never 1.0f unless is done
float OSAudioRecorder_encoderRelProgress(STOSAudioRecorder* obj){
	float r = 0.0f;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curEncoding != NULL){
			STAREncodingState* state = opq->curEncoding;
			if(!state->isRunning || state->samplesExpectedCount <= 0 || state->samplesProcessedCount >= state->samplesExpectedCount){
				r = 1.0f;
			} else {
				r = ((float)state->samplesProcessedCount / (float)state->samplesExpectedCount);
				if(r >= 1.0f) r = 0.99f;
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorder_encoderFinish(STOSAudioRecorder* obj){ //Cancel
	OSARBool r = FALSE;
	if(obj->opaque != NULL){
		STOSAudioRecorderOpq* opq = (STOSAudioRecorderOpq*)obj->opaque;
		if(opq->curEncoding != NULL){
			STAREncodingState* state = opq->curEncoding;
			//Stop thread
			while(state->isRunning){
				state->stopSignal = TRUE;
			}
			//Release
			{
				if(state->fileIn != NULL){
					fclose(state->fileIn);
					state->fileIn = NULL;
				}
				if(state->fileOut != NULL){
					fflush(state->fileOut);
					fclose(state->fileOut);
					state->fileOut = NULL;
				}
				if(state->thread != NULL){
					NBThread_release(state->thread);
					state->thread = NULL;
				}
				free(state);
				state = opq->curEncoding = NULL;
			}
			r = TRUE;
		}
	}
	return r;
}

//Opus

#include "opus/include/opus.h"

#define OPUS_MAX_PACKET (1500)

typedef struct STOSAudioRecorderEncoderOpusState_ {
	OpusEncoder*	encoder;
	unsigned char	packet[OPUS_MAX_PACKET + 257];
	int				channels;
	int				bitsPerSample;
	int				samplerate;
	int				blockAlign;
	int				samplesTotal;
} STOSAudioRecorderEncoderOpusState;

void* OSAudioRecorderEncoderOpusStart(FILE* dst, int channels, int bitsPerSample, int samplerate, int blockAlign, int samplesTotal){ 
	void* r = NULL;
	int err = 0;
	//int sampling_rates[5] = {8000, 12000, 16000, 24000, 48000};
	//int channels[2] = {1, 2};
	//int applications[3] = {OPUS_APPLICATION_AUDIO, OPUS_APPLICATION_VOIP, OPUS_APPLICATION_RESTRICTED_LOWDELAY};
	//int bitrates[11] = {6000, 12000, 16000, 24000, 32000, 48000, 64000, 96000, 510000, OPUS_AUTO, OPUS_BITRATE_MAX};
	//int frame_sizes_ms_x2[9] = {5, 10, 20, 40, 80, 120, 160, 200, 240};  /* x2 to avoid 2.5 ms */
	OpusEncoder* encoder = opus_encoder_create(samplerate, channels, OPUS_APPLICATION_AUDIO, &err);
	if(encoder == NULL){
		OSAR_PRINTF_ERROR("'opus_encoder_create' failed.\n");
	} else {
		/*if(opus_encoder_ctl(encoder, OPUS_SET_BITRATE(OPUS_AUTO)) != OPUS_OK){
			OSAR_PRINTF_ERROR("'OPUS_SET_BITRATE' failed.\n");
		}
		if(opus_encoder_ctl(enc, OPUS_SET_FORCE_CHANNELS(force_channel)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_VBR(vbr)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_VBR_CONSTRAINT(vbr_constraint)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(complexity)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_MAX_BANDWIDTH(max_bw)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_SIGNAL(sig)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(inband_fec)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(pkt_loss)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(lsb_depth)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_PREDICTION_DISABLED(pred_disabled)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_DTX(dtx)) != OPUS_OK) test_failed();
		if(opus_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(frame_size_enum)) != OPUS_OK) test_failed();*/
		{
			STOSAudioRecorderEncoderOpusState* state = (STOSAudioRecorderEncoderOpusState*)malloc(sizeof(STOSAudioRecorderEncoderOpusState));
			memset(state, 0, sizeof(*state));
			state->encoder		= encoder;
			state->channels		= channels;
			state->bitsPerSample = bitsPerSample;
			state->samplerate	= samplerate;
			state->blockAlign	= blockAlign;
			state->samplesTotal	= samplesTotal;
			//Consume
			r = state;
			encoder = NULL;
		}
		//Release (if not consumed)
		if(encoder != NULL){
			opus_encoder_destroy(encoder);
			encoder = NULL;
		}
	}
	return r;
}

OSARBool OSAudioRecorderEncoderOpusFeed(void* userData, FILE* dst, void* samples, long samplesBytes, long samplesCount){
	OSARBool r = FALSE;
	STOSAudioRecorderEncoderOpusState* state = (STOSAudioRecorderEncoderOpusState*)userData;
	if(state != NULL){
		OpusEncoder* encoder = state->encoder;
		if(encoder != NULL){
			//ToDo: asuming 16-bits, 1-channel samples
			opus_int16* data = (opus_int16*)samples;
			opus_int32 len = 0; long iSample = 0;
			r = TRUE;
			while(iSample < samplesCount){
				len = opus_encode(encoder, &data[iSample], (int)samplesCount, state->packet, OPUS_MAX_PACKET);
				if(len < 0 || len > OPUS_MAX_PACKET){
					OSAR_PRINTF_ERROR("'opus_encode' failed.\n");
					r = FALSE;
					break;
				} else if(fwrite(state->packet, len, 1, dst) != 1){
					OSAR_PRINTF_ERROR("'opus'/fwrite failed.\n");
					r = FALSE;
					break;
				} else {
					iSample += samplesCount;
				}
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorderEncoderOpusEnd(void* userData, FILE* dst){
	OSARBool r = FALSE;
	STOSAudioRecorderEncoderOpusState* state = (STOSAudioRecorderEncoderOpusState*)userData;
	if(state != NULL){
		OpusEncoder* encoder = state->encoder;
		if(encoder != NULL){
			r = TRUE;
			opus_encoder_destroy(encoder);
			state->encoder = encoder = NULL;
		}
		free(state);
		state = NULL;
	}
	return r;
}

//Flac

#include "flac/include/FLAC/stream_encoder.h"

typedef struct STOSAudioRecorderEncoderFlacState_ {
	FLAC__StreamEncoder* encoder;
	FLAC__int32*	flacBuff; //Required to convert packed data into libFLAC data.
	int				flacBuffSz;
	int				channels;
	int				bitsPerSample;
	int				samplerate;
	int				blockAlign;
	int				samplesTotal;
} STOSAudioRecorderEncoderFlacState;

void* OSAudioRecorderEncoderFlacStart(FILE* dst, int channels, int bitsPerSample, int samplerate, int blockAlign, int samplesTotal){
	void* r = NULL;
	FLAC__StreamEncoder* encoder = FLAC__stream_encoder_new();
	if(encoder == NULL){
		OSAR_PRINTF_ERROR("'FLAC__stream_encoder_new' failed.\n");
	} else {
		if(!FLAC__stream_encoder_set_verify(encoder, true)){
			OSAR_PRINTF_ERROR("'FLAC__stream_encoder_set_verify' failed.\n");
		} else if(!FLAC__stream_encoder_set_compression_level(encoder, 5)){
			OSAR_PRINTF_ERROR("'FLAC__stream_encoder_set_compression_level(5)' failed.\n");
		} else if(!FLAC__stream_encoder_set_channels(encoder, channels)){
			OSAR_PRINTF_ERROR("'FLAC__stream_encoder_set_channels(%d)' failed.\n", channels);
		} else if(!FLAC__stream_encoder_set_bits_per_sample(encoder, bitsPerSample)){
			OSAR_PRINTF_ERROR("'FLAC__stream_encoder_set_bits_per_sample(%d)' failed.\n", bitsPerSample);
		} else if(!FLAC__stream_encoder_set_sample_rate(encoder, samplerate)){
			OSAR_PRINTF_ERROR("'FLAC__stream_encoder_set_sample_rate(%d)' failed.\n", samplerate);
		} else if(!FLAC__stream_encoder_set_total_samples_estimate(encoder, samplesTotal)){
			OSAR_PRINTF_ERROR("'FLAC__stream_encoder_set_total_samples_estimate(%d)' failed.\n", samplesTotal);
		} else if(FLAC__stream_encoder_init_FILE(encoder, dst, NULL, NULL) != FLAC__STREAM_ENCODER_INIT_STATUS_OK){
			OSAR_PRINTF_ERROR("'FLAC__stream_encoder_init_FILE' failed.\n");
		} else {
			STOSAudioRecorderEncoderFlacState* state = (STOSAudioRecorderEncoderFlacState*)malloc(sizeof(STOSAudioRecorderEncoderFlacState));
			memset(state, 0, sizeof(*state));
			state->encoder		= encoder;
			state->channels		= channels;
			state->bitsPerSample = bitsPerSample;
			state->samplerate	= samplerate;
			state->blockAlign	= blockAlign;
			state->samplesTotal	= samplesTotal;
			//Consume
			r = state;
			encoder = NULL;
		}
		//Release (if not consumed)
		if(encoder != NULL){
			FLAC__stream_encoder_delete(encoder);
			encoder = NULL;
		}
	}
	return r;
}

OSARBool OSAudioRecorderEncoderFlacFeed(void* userData, FILE* dst, void* samples, long samplesBytes, long samplesCount){
	OSARBool r = FALSE;
	STOSAudioRecorderEncoderFlacState* state = (STOSAudioRecorderEncoderFlacState*)userData;
	if(state != NULL){
		FLAC__StreamEncoder* encoder = state->encoder;
		if(encoder != NULL){
			//Convert packed data into libFLAC data.
			{
				if(state->flacBuffSz < samplesCount){
					state->flacBuffSz = (int)samplesCount;
					if(state->flacBuff != NULL){
						free(state->flacBuff);
						state->flacBuff = NULL;
					}
					state->flacBuff = (FLAC__int32*)malloc(sizeof(FLAC__int32) * state->flacBuffSz);
				}
				//Copy
				{
					//ToDo: asuming 16-bits, 1-channel samples
					FLAC__int32* dst = state->flacBuff;
					FLAC__int16 sample = 0;
					unsigned char* sampleBytes = (unsigned char*)&sample;
					unsigned char* data = (unsigned char*)samples;
					int i; for(i = 0; i < samplesCount; i++){
						sampleBytes[0]	= data[(i * 2)];
						sampleBytes[1]	= data[(i * 2) + 1];
						dst[i]			= (FLAC__int32)sample;
					}
				}
			}
			//
			if(!FLAC__stream_encoder_process_interleaved(encoder, state->flacBuff, (int)samplesCount)){ //For some reason, FLAC expects 32 bits number
				OSAR_PRINTF_ERROR("'FLAC__stream_encoder_process(%d)' failed.\n", (int)samplesCount);
			} else {
				r = TRUE;
			}
		}
	}
	return r;
}

OSARBool OSAudioRecorderEncoderFlacEnd(void* userData, FILE* dst){
	OSARBool r = FALSE;
	STOSAudioRecorderEncoderFlacState* state = (STOSAudioRecorderEncoderFlacState*)userData;
	if(state != NULL){
		FLAC__StreamEncoder* encoder = state->encoder;
		if(encoder != NULL){
			if(!FLAC__stream_encoder_finish(encoder)){
				OSAR_PRINTF_ERROR("'FLAC__stream_encoder_finish' failed.\n");
			} else {
				r = TRUE;
			}
			FLAC__stream_encoder_delete(encoder);
			state->encoder = encoder = NULL;
		}
		if(state->flacBuff != NULL){
			free(state->flacBuff);
			state->flacBuff = NULL;
			state->flacBuffSz = 0;
		}
		free(state);
		state = NULL;
	}
	return r;
}

//end-of-file
