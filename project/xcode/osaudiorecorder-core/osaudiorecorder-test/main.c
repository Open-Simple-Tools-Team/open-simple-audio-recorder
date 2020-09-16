//
//  main.c
//  osaudiorecorder-test
//
//  Created by Marcos Ortega on 9/8/20.
//

#include <stdio.h>
#include <unistd.h>	//sleep()
#include <string.h>	//memset
#include "../../../../include/osaudiorecorder.h"
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
	//Record and encoding test
	if(TRUE){
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
	}
	//Encoding test
	if(TRUE){
		STOSAudioRecorder rec;
		if(!OSAudioRecorder_init(&rec)){
			printf("ERROR, 'OSAudioRecorder_init' failed.\n");
		} else {
			const char* filepath = "/Users/mortegam/Downloads/_audio.wav";
			//Encode FLAC
			{
				const char* filepathDst = "/Users/mortegam/Downloads/_audio_mine.flac";
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
				const char* filepathDst = "/Users/mortegam/Downloads/_audio_mine.opus";
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
			OSAudioRecorder_release(&rec);
		}
	}
	//Wait for debugging tools
	/*while(TRUE){
		sleep(1);
	}*/
	//
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
