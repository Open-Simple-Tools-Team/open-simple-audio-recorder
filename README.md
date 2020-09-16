# open-simple-audio-recorder
App and core for audio recording with Open and Royalty-free encoders and software.

# Core
The core is a C library and supports the inclussion of encoders as plugins.

The main code is designed in atomic-files, only one ".h" header and one ".c" source encapsulates the logic of the library.

External code (flac, libogg, nixtla-audio and opus) brings their own structure, but should be as easy to "uncompress" and compile the main project. No need to compile those libraries individually.

# Apps and platforms
Currently an Android app with android-studio project is available. A MacOS command tool with a Xcode project is available.

The code is intended to be universal, and should be a step or two away from compiling to other platforms, like iOS, Windows or Linux.

# History
(Sept/2020) This project was originally commisioned by Inti Luna to Marcos Ortega as a tool for audio recording compatibe with the Open Data Kit (ODK) Collect Android App.

# How to use the core in your Android project
Copy these files into your project:

Java header
```/project/android-studio/app/src/main/java/org/ostteam/audiorecorder/Core.java
```

JNI libraries
```/project/android-studio/app/src/libs/arm64-v8a/libosaudiorecorder-core.so
/project/android-studio/app/src/libs/armeabi-v7a/libosaudiorecorder-core.so
/project/android-studio/app/src/libs/x86/libosaudiorecorder-core.so
/project/android-studio/app/src/libs/x86_64/libosaudiorecorder-core.so
```

Add permissions to your AndroidManifest.xml:

```<uses-permission android:name="android.permission.MODIFY_AUDIO_SETTINGS"/>
<uses-permission android:name="android.permission.RECORD_AUDIO"/>
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
```

Set your build.gradle to inlcude the libraries:

```android {
    sourceSets {
        main {
            jniLibs.srcDir 'src/libs'
        }
    }
}
```

Uset the Core methods as in our Android project. We hope the library methods are intitutive for you.

# How to use the core in your C, C++ or JNI project

Compile or use our already compiled libraries.

Audio capture example:

```STOSAudioRecorder rec;
if(OSAudioRecorder_init(&rec)){
	const char* filepath = "test2.wav";
	if(OSAudioRecorder_start(&rec, filepath, 30, FALSE)){
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
	}
	OSAudioRecorder_release(&rec);
}
```

Audio encoder example:
Important: the player/encoder are intended for WAV files produced by the recorder. This is not an universal player/encoder.

```STOSAudioRecorder rec;
if(OSAudioRecorder_init(&rec)){
	const char* filepath = "test2.wav";
	//Encode FLAC
	{
		const char* filepathDst = "test2.flac";
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
		const char* filepathDst = "test2.opus";
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
```

# How to add new encoders

Define these three methods:

```void* myEncoderStart(FILE* dst, int channels, int bitsPerSample, int samplerate, int blockAlign, int samplesTotal){
	void* myStateData = NULL;
	//Allocate your state data, init and write the necesary header to the file.
	//Keep a copy of channels, bitsPerSample, samplerate, blockAlign and samplesTotal values if necesary.
	return myStateData;
}

OSARBool myEncoderFeed(void* userData, FILE* dst, void* samples, long samplesBytes, long samplesCount){
	OSARBool r = FALSE;
	void* myStateData = userData;
	//Write the PCM samples to the file.
	return r;
}

OSARBool myEncoderEnd(void* userData, FILE* dst){
	OSARBool r = FALSE;
	void* myStateData = userData;
	//do final writes to file and deallocate your state data.
	return r;
}
```
Register your encoder:

```OSAudioRecorder_encoderAdd(obj, "myEncoderId", myEncoderStart, myEncoderFeed, myEncoderEnd);
```

