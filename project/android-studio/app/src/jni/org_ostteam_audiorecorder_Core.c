
#include "org_ostteam_audiorecorder_Core.h"
#include <android/log.h>
//
#include <stdio.h>
#include <stdlib.h>	//malloc
#include <string.h>	//memset
#include "../../../../../src/osaudiorecorder.h"

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_engineInit(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore == NULL){
			nCore = (STOSAudioRecorder*)malloc(sizeof(STOSAudioRecorder));
			if(OSAudioRecorder_init(nCore)){
				(*jEnv)->SetLongField(jEnv, jObj, jCoreId, (jlong)nCore);
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT void JNICALL Java_org_ostteam_audiorecorder_Core_engineRelease(JNIEnv *jEnv, jobject jObj){
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			OSAudioRecorder_release(nCore);
			free(nCore);
			(*jEnv)->SetLongField(jEnv, jObj, jCoreId, (jlong)0);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
}

JNIEXPORT void JNICALL Java_org_ostteam_audiorecorder_Core_tick(JNIEnv *jEnv, jobject jObj){
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			OSAudioRecorder_tick(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_start(JNIEnv *jEnv, jobject jObj, jstring jWavFilepath, jint buffersPerSec, jboolean keepAvgsOfSamples){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			const char* utfWavFilepath = (*jEnv)->GetStringUTFChars(jEnv, jWavFilepath, 0);
			if(OSAudioRecorder_start(nCore, utfWavFilepath, buffersPerSec, keepAvgsOfSamples)){
				r = JNI_TRUE;
			}
			(*jEnv)->ReleaseStringUTFChars(jEnv, jWavFilepath, utfWavFilepath);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_pause(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_pause(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_resume(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_resume(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_stop(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_stop(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_reset(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_reset(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_isActive(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_isActive(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_isRecording(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_isRecording(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_isPaused(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_isPaused(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jlong JNICALL Java_org_ostteam_audiorecorder_Core_samplesCount(JNIEnv *jEnv, jobject jObj){
	jlong r = 0;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = (jlong)OSAudioRecorder_samplesCount(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jlong JNICALL Java_org_ostteam_audiorecorder_Core_samplesBytes(JNIEnv *jEnv, jobject jObj){
	jlong r = 0;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = (jlong)OSAudioRecorder_samplesBytes(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jfloat JNICALL Java_org_ostteam_audiorecorder_Core_samplesSecs(JNIEnv *jEnv, jobject jObj){
	jfloat r = 0;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = (jfloat)OSAudioRecorder_samplesSecs(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jfloat JNICALL Java_org_ostteam_audiorecorder_Core_samplesRelAvg(JNIEnv *jEnv, jobject jObj, jboolean resetAvg){
	jfloat r = 0.0f;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = (jfloat)OSAudioRecorder_samplesRelAvg(nCore, resetAvg);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

//player

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_playerLoad(JNIEnv *jEnv, jobject jObj, jstring jFilepath, jint buffersPerSec){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			const char* utfFilepath = (*jEnv)->GetStringUTFChars(jEnv, jFilepath, 0);
			if(OSAudioRecorder_playerLoad(nCore, utfFilepath, buffersPerSec)){
				r = JNI_TRUE;
			}
			(*jEnv)->ReleaseStringUTFChars(jEnv, jFilepath, utfFilepath);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_playerPlay(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_playerPlay(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_playerPause(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_playerPause(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_playerStop(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_playerStop(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_playerIsLoaded(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_playerIsLoaded(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_playerIsPaused(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_playerIsPaused(nCore)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_playerSeekRelPos(JNIEnv *jEnv, jobject jObj, jfloat relPos){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			if(OSAudioRecorder_playerSeekRelPos(nCore, relPos)){
				r = JNI_TRUE;
			}
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jfloat JNICALL Java_org_ostteam_audiorecorder_Core_playerRelProgress(JNIEnv *jEnv, jobject jObj){
	jfloat r = 0.0f;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = OSAudioRecorder_playerRelProgress(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jfloat JNICALL Java_org_ostteam_audiorecorder_Core_playerSecsTotal(JNIEnv *jEnv, jobject jObj){
	jfloat r = 0.0f;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = OSAudioRecorder_playerSecsTotal(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

//encoder

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_encoderStart(JNIEnv *jEnv, jobject jObj, jstring jFilepathIn, jstring jFilepathOut, jstring jFormatId){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			const char* utfFilepathIn	= (*jEnv)->GetStringUTFChars(jEnv, jFilepathIn, 0);
			const char* utfFilepathOut	= (*jEnv)->GetStringUTFChars(jEnv, jFilepathOut, 0);
			const char* utfFormatId		= (*jEnv)->GetStringUTFChars(jEnv, jFormatId, 0);
			if(OSAudioRecorder_encoderStart(nCore, utfFilepathIn,utfFilepathOut, utfFormatId)){
				r = JNI_TRUE;
			}
			(*jEnv)->ReleaseStringUTFChars(jEnv, jFormatId, utfFormatId);
			(*jEnv)->ReleaseStringUTFChars(jEnv, jFilepathOut, utfFilepathOut);
			(*jEnv)->ReleaseStringUTFChars(jEnv, jFilepathIn, utfFilepathIn);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_encoderIsLoaded(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = OSAudioRecorder_encoderIsLoaded(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jfloat JNICALL Java_org_ostteam_audiorecorder_Core_encoderRelProgress(JNIEnv *jEnv, jobject jObj){
	jfloat r = 0.0f;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = OSAudioRecorder_encoderRelProgress(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

JNIEXPORT jboolean JNICALL Java_org_ostteam_audiorecorder_Core_encoderFinish(JNIEnv *jEnv, jobject jObj){
	jboolean r = JNI_FALSE;
	jclass jClsCore = (*jEnv)->FindClass(jEnv, "org/ostteam/audiorecorder/Core");
	if(jClsCore != NULL){
		jfieldID jCoreId = (*jEnv)->GetFieldID(jEnv, jClsCore, "_core", "J");
		STOSAudioRecorder* nCore = (STOSAudioRecorder*)(*jEnv)->GetLongField(jEnv, jObj, jCoreId);
		if(nCore != NULL){
			r = OSAudioRecorder_encoderFinish(nCore);
		}
		(*jEnv)->DeleteLocalRef(jEnv, jClsCore);
	}
	return r;
}

// ---------------------------------
// --- Metodos callbacks
// ---------------------------------

/*
Tipos para los methods descriptors
BaseType Character	 Type	 Interpretation
B	 byte	 signed byte
C	 char	 Unicode character
D	 double	 double-precision floating-point value
F	 float	 single-precision floating-point value
I	 int	 integer
J	 long	 long integer
L<classname>;	 reference	 an instance of class <classname>
S	 short	 signed short
Z	 boolean	 true or false
[	 reference	 one array dimension
Ljava/lang/String;
 */

//


