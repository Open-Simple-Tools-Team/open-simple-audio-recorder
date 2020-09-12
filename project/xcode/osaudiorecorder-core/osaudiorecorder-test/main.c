//
//  main.c
//  osaudiorecorder-test
//
//  Created by Marcos Ortega on 9/8/20.
//

#include <stdio.h>
#include <unistd.h>	//sleep()
#include <string.h>	//memset
#include "../../../../src/osaudiorecorder.h"
//#include "../../../../src/nixtla-audio/nixtla-audio.h"

/*void test_CaptureBufferFilledCallback(STNix_Engine* engAbs, void* userdata, const STNix_audioDesc audioDesc, const NixUI8* audioData, const NixUI32 audioDataBytes, const NixUI32 audioDataSamples);

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
} STARRiffWaveChunckFmt;*/

int main(int argc, const char * argv[]) {
	// insert code here...
	printf("Start-of-program....\n");
	{
		STOSAudioRecorder rec;
		if(!OSAudioRecorder_init(&rec)){
			printf("ERROR, 'OSAudioRecorder_init' failed.\n");
		} else {
			const char* filepath = "/Users/mortegam/Desktop/test2.wav";
			if(!OSAudioRecorder_start(&rec, filepath, 30, FALSE)){
				printf("ERROR, 'OSAudioRecorder_start' failed.\n");
			} else {
				//Wait few seconds
				{
					unsigned long long slept = 0;
					while(slept < (5 * 1000000)){
						usleep(100000);
						slept += 100000; 
						OSAudioRecorder_tick(&rec);
					}
				}
				OSAudioRecorder_stop(&rec);
				//Encode FLAC
				{
					const char* filepathDst = "/Users/mortegam/Desktop/test2.flac";
					if(!OSAudioRecorder_encoderStart(&rec, filepath, filepathDst, "flac")){
						printf("ERROR, 'OSAudioRecorder_encoderStart' failed.\n");
					} else {
						//Wait few seconds
						float prog = 0.0f;
						while(OSAudioRecorder_encoderIsLoaded(&rec)){
							usleep(100000);
							prog = OSAudioRecorder_encoderRelProgress(&rec); //never 1.0f unless done
							printf("INFO, 'OSAudioRecorder_encoderRelProgress': %d%%.\n", (int)(prog * 100.0f));
							if(prog >= 1.0f){
								break;
							}
						}
						OSAudioRecorder_encoderFinish(&rec);
					}
				}
				//Encode OPUS
				{
					const char* filepathDst = "/Users/mortegam/Desktop/test2.opus";
					if(!OSAudioRecorder_encoderStart(&rec, filepath, filepathDst, "opus")){
						printf("ERROR, 'OSAudioRecorder_encoderStart' failed.\n");
					} else {
						//Wait few seconds
						float prog = 0.0f;
						while(OSAudioRecorder_encoderIsLoaded(&rec)){
							usleep(100000);
							prog = OSAudioRecorder_encoderRelProgress(&rec); //never 1.0f unless done
							printf("INFO, 'OSAudioRecorder_encoderRelProgress': %d%%.\n", (int)(prog * 100.0f));
							if(prog >= 1.0f){
								break;
							}
						}
						OSAudioRecorder_encoderFinish(&rec);
					}
				}
			}
			OSAudioRecorder_release(&rec);
		}
		/*STNix_Engine nix;
		NixUI32 capabilities;
		if(!nixInit(&nix, 0)){
			printf("ERROR, 'nixInit' failed.\n");
		} else {
			nixContextActivate(&nix); //good practice (does nothing for OpenSL/Android)
			//Eval capabilities
			capabilities = nixCapabilities(&nix);
			if((capabilities & NIX_CAP_AUDIO_CAPTURE) == 0){
				printf("ERROR, current device cannot capture audio.\n");
			} else {
				const char* filepath = "/Users/mortegam/Desktop/test.wav";
				FILE* file = fopen(filepath, "wb+");
				if(file == NULL){
					printf("ERROR, 'fopen' failed for path '%s'.\n", filepath);
				} else {
					const STNix_audioDesc capDesc = {
						.samplesFormat	= ENNix_sampleFormat_int,
						.channels		= 1,
						.bitsPerSample	= 16,
						.samplerate		= 22050,
						.blockAlign		= (16 / 8) * 1 //ToDo: try zero
					};
					const NixUI16 buffersCount = 32;
					const NixUI16 samplesPerBuffer = (capDesc.samplerate / buffersCount);
					long waveSzPos = 0, dataHeadPos = 0;
					//Write WAV/RIFF header
					{
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
							fmtHead.format			= (capDesc.samplesFormat == ENNix_sampleFormat_float ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM);
							fmtHead.channels		= capDesc.channels;
							fmtHead.samplesPerSec	= capDesc.samplerate;
							fmtHead.avgBytesPerSec	= (capDesc.samplerate * capDesc.bitsPerSample * capDesc.channels) / 8;
							fmtHead.blocksAlign		= capDesc.blockAlign;
							fmtHead.bitsPerSample	= capDesc.bitsPerSample;
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
					if(!nixCaptureInit(&nix, &capDesc, buffersCount, samplesPerBuffer, &test_CaptureBufferFilledCallback, &file)){
						printf("ERROR, could not start audio capture.\n");
					} else {
						//Start
						nixCaptureStart(&nix);
						//Wait few seconds
						{
							unsigned long long slept = 0;
							while(slept < (5 * 1000000)){
								usleep(100000);
								slept += 100000; 
								nixTick(&nix);
							}
						}
						//End
						nixCaptureStop(&nix);
						//update the size fo the 'data' sub-chunk
						{
							long finalPos = ftell(file);
							if(dataHeadPos < finalPos){
								const NixUI32 dataSz = (NixUI32)(finalPos - dataHeadPos - sizeof(STARRiffWaveChunckHead));
								//Write 'data' padding byte
								if((dataSz % 2) != 0){
									const char padd = '\0';
									fwrite(&padd, sizeof(padd), 1, file);
									finalPos++;
								}
								fseek(file, dataHeadPos + 4, SEEK_SET);
								fwrite(&dataSz, sizeof(dataSz), 1, file);
								fflush(file);
								//Update the size of the 'WAVE' chunk
								if(finalPos > 8){
									NixUI32 fileSz = (NixUI32)(finalPos - 8);
									fseek(file, waveSzPos, SEEK_SET);
									fwrite(&fileSz, sizeof(fileSz), 1, file);
									fflush(file);
								}
							}
						}
					}
					fclose(file);
					file = NULL;
				}
				
			}
			nixFinalize(&nix);
		}*/
	}
	printf("...end-of-program.\n");
	return 0;
}

/*void test_CaptureBufferFilledCallback(STNix_Engine* engAbs, void* userdata, const STNix_audioDesc audioDesc, const NixUI8* audioData, const NixUI32 audioDataBytes, const NixUI32 audioDataSamples){
	//Write wave data
	FILE** file = (FILE**)userdata;
	if(file != NULL && audioData != NULL && audioDataBytes > 0){
		if(*file != NULL){
			fwrite(audioData, audioDataBytes, 1, *file);
			fflush(*file);
		}
	}
}*/
