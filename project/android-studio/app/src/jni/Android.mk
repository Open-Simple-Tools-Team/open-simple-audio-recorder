
LOCAL_PATH    := $(call my-dir)

AR_INC_ROOT  := ../../../../src
AR_SRC_ROOT  := ../../../../../src

#------------------------------
#- CORE OPUS (static library) -
#------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE		:= osaudiorecorder-core-opus

LOCAL_C_INCLUDES	:=
LOCAL_SRC_FILES		:= 

LOCAL_CFLAGS		+= -DENABLE_HARDENING=1 -DHAVE_DLFCN_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_LRINT=1 -DHAVE_LRINTF=1 -DHAVE_MEMORY_H=1 -DHAVE_STDINT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRINGS_H=1 -DHAVE_STRING_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DLT_OBJDIR="\".libs/\"" -DOPUS_BUILD -DPACKAGE_BUGREPORT="\"opus@xiph.org\"" -DPACKAGE_NAME="\"opus\"" -DPACKAGE_STRING="\"opus 1.3.1\"" -DPACKAGE_TARNAME="\"opus\"" -DPACKAGE_URL="\"\"" -DPACKAGE_VERSION="\"1.3.1\"" -DSTDC_HEADERS=1 -DVAR_ARRAYS=1 -Drestrict=__restrict

LOCAL_C_INCLUDES	+= \
  $(AR_INC_ROOT)/opus \
  $(AR_INC_ROOT)/opus/include/ \
  $(AR_INC_ROOT)/opus/celt/ \
  $(AR_INC_ROOT)/opus/silk/ \
  $(AR_INC_ROOT)/opus/silk/float \
  $(AR_INC_ROOT)/opus/silk/fixed

#xcode
# -DENABLE_HARDENING=1 -DHAVE_DLFCN_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_LRINT=1 -DHAVE_LRINTF=1 -DHAVE_MEMORY_H=1 -DHAVE_STDINT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRINGS_H=1 -DHAVE_STRING_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DLT_OBJDIR="\".libs/\"" -DOPUS_BUILD -DPACKAGE_BUGREPORT="\"opus@xiph.org\"" -DPACKAGE_NAME="\"opus\"" -DPACKAGE_STRING="\"opus 1.3.1\"" -DPACKAGE_TARNAME="\"opus\"" -DPACKAGE_URL="\"\"" -DPACKAGE_VERSION="\"1.3.1\"" -DSTDC_HEADERS=1 -DVAR_ARRAYS=1 -Drestrict=__restrict -I$(PROJECT_DIR)/../../../src/opus -I$(PROJECT_DIR)/../../../src/opus/include/ -I$(PROJECT_DIR)/../../../src/opus/celt/ -I$(PROJECT_DIR)/../../../src/opus/celt/x86/ -I$(PROJECT_DIR)/../../../src/opus/silk/ -I$(PROJECT_DIR)/../../../src/opus/silk/float -I$(PROJECT_DIR)/../../../src/opus/silk/fixed

#armeabi-v7a, arm64-v8a, x86, x86_64

ifneq (,$(findstring arm,$(TARGET_ARCH_ABI)))
  #ARM specifics
  LOCAL_C_INCLUDES	+= \
    $(AR_INC_ROOT)/opus/celt/arm/  
  LOCAL_SRC_FILES	+= \
    $(AR_SRC_ROOT)/opus/celt/arm/arm_celt_map.c \
    $(AR_SRC_ROOT)/opus/celt/arm/armcpu.c \
    $(AR_SRC_ROOT)/opus/celt/arm/celt_mdct_ne10.c \
    $(AR_SRC_ROOT)/opus/celt/arm/celt_neon_intr.c \
    $(AR_SRC_ROOT)/opus/celt/arm/pitch_neon_intr.c
  LOCAL_SRC_FILES	+= \
    $(AR_SRC_ROOT)/opus/silk/arm/arm_silk_map.c  \
    $(AR_SRC_ROOT)/opus/silk/arm/biquad_alt_neon_intr.c \
    $(AR_SRC_ROOT)/opus/silk/arm/LPC_inv_pred_gain_neon_intr.c \
    $(AR_SRC_ROOT)/opus/silk/arm/NSQ_del_dec_neon_intr.c \
    $(AR_SRC_ROOT)/opus/silk/arm/NSQ_neon.c
else ifneq (,$(findstring x86,$(TARGET_ARCH_ABI)))
  #x86 specifics
  LOCAL_C_INCLUDES	+= \
    $(AR_INC_ROOT)/opus/celt/x86/
  LOCAL_SRC_FILES	+= \
    $(AR_SRC_ROOT)/opus/celt/x86/x86cpu.c \
    $(AR_SRC_ROOT)/opus/celt/x86/x86_celt_map.c \
    $(AR_SRC_ROOT)/opus/celt/x86/pitch_sse.c \
    $(AR_SRC_ROOT)/opus/celt/x86/pitch_sse2.c \
    $(AR_SRC_ROOT)/opus/celt/x86/vq_sse2.c \
    $(AR_SRC_ROOT)/opus/celt/x86/celt_lpc_sse4_1.c \
    $(AR_SRC_ROOT)/opus/celt/x86/pitch_sse4_1.c
  LOCAL_SRC_FILES	+= \
    $(AR_SRC_ROOT)/opus/silk/x86/NSQ_sse4_1.c \
    $(AR_SRC_ROOT)/opus/silk/x86/NSQ_del_dec_sse4_1.c \
    $(AR_SRC_ROOT)/opus/silk/x86/x86_silk_map.c \
    $(AR_SRC_ROOT)/opus/silk/x86/VAD_sse4_1.c \
    $(AR_SRC_ROOT)/opus/silk/x86/VQ_WMat_EC_sse4_1.c
endif

LOCAL_SRC_FILES		+= \
  $(AR_SRC_ROOT)/opus/celt/bands.c \
  $(AR_SRC_ROOT)/opus/celt/celt.c \
  $(AR_SRC_ROOT)/opus/celt/celt_encoder.c \
  $(AR_SRC_ROOT)/opus/celt/celt_decoder.c \
  $(AR_SRC_ROOT)/opus/celt/cwrs.c \
  $(AR_SRC_ROOT)/opus/celt/entcode.c \
  $(AR_SRC_ROOT)/opus/celt/entdec.c \
  $(AR_SRC_ROOT)/opus/celt/entenc.c \
  $(AR_SRC_ROOT)/opus/celt/kiss_fft.c \
  $(AR_SRC_ROOT)/opus/celt/laplace.c \
  $(AR_SRC_ROOT)/opus/celt/mathops.c \
  $(AR_SRC_ROOT)/opus/celt/mdct.c \
  $(AR_SRC_ROOT)/opus/celt/modes.c \
  $(AR_SRC_ROOT)/opus/celt/pitch.c \
  $(AR_SRC_ROOT)/opus/celt/celt_lpc.c \
  $(AR_SRC_ROOT)/opus/celt/quant_bands.c \
  $(AR_SRC_ROOT)/opus/celt/rate.c \
  $(AR_SRC_ROOT)/opus/celt/vq.c \
  $(AR_SRC_ROOT)/opus/silk/CNG.c \
  $(AR_SRC_ROOT)/opus/silk/code_signs.c \
  $(AR_SRC_ROOT)/opus/silk/init_decoder.c \
  $(AR_SRC_ROOT)/opus/silk/decode_core.c \
  $(AR_SRC_ROOT)/opus/silk/decode_frame.c \
  $(AR_SRC_ROOT)/opus/silk/decode_parameters.c \
  $(AR_SRC_ROOT)/opus/silk/decode_indices.c \
  $(AR_SRC_ROOT)/opus/silk/decode_pulses.c \
  $(AR_SRC_ROOT)/opus/silk/decoder_set_fs.c \
  $(AR_SRC_ROOT)/opus/silk/dec_API.c \
  $(AR_SRC_ROOT)/opus/silk/enc_API.c \
  $(AR_SRC_ROOT)/opus/silk/encode_indices.c \
  $(AR_SRC_ROOT)/opus/silk/encode_pulses.c \
  $(AR_SRC_ROOT)/opus/silk/gain_quant.c \
  $(AR_SRC_ROOT)/opus/silk/interpolate.c \
  $(AR_SRC_ROOT)/opus/silk/LP_variable_cutoff.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF_decode.c \
  $(AR_SRC_ROOT)/opus/silk/NSQ.c \
  $(AR_SRC_ROOT)/opus/silk/NSQ_del_dec.c \
  $(AR_SRC_ROOT)/opus/silk/PLC.c \
  $(AR_SRC_ROOT)/opus/silk/shell_coder.c \
  $(AR_SRC_ROOT)/opus/silk/tables_gain.c \
  $(AR_SRC_ROOT)/opus/silk/tables_LTP.c \
  $(AR_SRC_ROOT)/opus/silk/tables_NLSF_CB_NB_MB.c \
  $(AR_SRC_ROOT)/opus/silk/tables_NLSF_CB_WB.c \
  $(AR_SRC_ROOT)/opus/silk/tables_other.c \
  $(AR_SRC_ROOT)/opus/silk/tables_pitch_lag.c \
  $(AR_SRC_ROOT)/opus/silk/tables_pulses_per_block.c \
  $(AR_SRC_ROOT)/opus/silk/VAD.c \
  $(AR_SRC_ROOT)/opus/silk/control_audio_bandwidth.c \
  $(AR_SRC_ROOT)/opus/silk/quant_LTP_gains.c \
  $(AR_SRC_ROOT)/opus/silk/VQ_WMat_EC.c \
  $(AR_SRC_ROOT)/opus/silk/HP_variable_cutoff.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF_encode.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF_VQ.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF_unpack.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF_del_dec_quant.c \
  $(AR_SRC_ROOT)/opus/silk/process_NLSFs.c \
  $(AR_SRC_ROOT)/opus/silk/stereo_LR_to_MS.c \
  $(AR_SRC_ROOT)/opus/silk/stereo_MS_to_LR.c \
  $(AR_SRC_ROOT)/opus/silk/check_control_input.c \
  $(AR_SRC_ROOT)/opus/silk/control_SNR.c \
  $(AR_SRC_ROOT)/opus/silk/init_encoder.c \
  $(AR_SRC_ROOT)/opus/silk/control_codec.c \
  $(AR_SRC_ROOT)/opus/silk/A2NLSF.c \
  $(AR_SRC_ROOT)/opus/silk/ana_filt_bank_1.c \
  $(AR_SRC_ROOT)/opus/silk/biquad_alt.c \
  $(AR_SRC_ROOT)/opus/silk/bwexpander_32.c \
  $(AR_SRC_ROOT)/opus/silk/bwexpander.c \
  $(AR_SRC_ROOT)/opus/silk/debug.c \
  $(AR_SRC_ROOT)/opus/silk/decode_pitch.c \
  $(AR_SRC_ROOT)/opus/silk/inner_prod_aligned.c \
  $(AR_SRC_ROOT)/opus/silk/lin2log.c \
  $(AR_SRC_ROOT)/opus/silk/log2lin.c \
  $(AR_SRC_ROOT)/opus/silk/LPC_analysis_filter.c \
  $(AR_SRC_ROOT)/opus/silk/LPC_inv_pred_gain.c \
  $(AR_SRC_ROOT)/opus/silk/table_LSF_cos.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF2A.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF_stabilize.c \
  $(AR_SRC_ROOT)/opus/silk/NLSF_VQ_weights_laroia.c \
  $(AR_SRC_ROOT)/opus/silk/pitch_est_tables.c \
  $(AR_SRC_ROOT)/opus/silk/resampler.c \
  $(AR_SRC_ROOT)/opus/silk/resampler_down2_3.c \
  $(AR_SRC_ROOT)/opus/silk/resampler_down2.c \
  $(AR_SRC_ROOT)/opus/silk/resampler_private_AR2.c \
  $(AR_SRC_ROOT)/opus/silk/resampler_private_down_FIR.c \
  $(AR_SRC_ROOT)/opus/silk/resampler_private_IIR_FIR.c \
  $(AR_SRC_ROOT)/opus/silk/resampler_private_up2_HQ.c \
  $(AR_SRC_ROOT)/opus/silk/resampler_rom.c \
  $(AR_SRC_ROOT)/opus/silk/sigm_Q15.c \
  $(AR_SRC_ROOT)/opus/silk/sort.c \
  $(AR_SRC_ROOT)/opus/silk/sum_sqr_shift.c \
  $(AR_SRC_ROOT)/opus/silk/stereo_decode_pred.c \
  $(AR_SRC_ROOT)/opus/silk/stereo_encode_pred.c \
  $(AR_SRC_ROOT)/opus/silk/stereo_find_predictor.c \
  $(AR_SRC_ROOT)/opus/silk/stereo_quant_pred.c \
  $(AR_SRC_ROOT)/opus/silk/LPC_fit.c \
  $(AR_SRC_ROOT)/opus/silk/float/apply_sine_window_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/corrMatrix_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/encode_frame_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/find_LPC_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/find_LTP_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/find_pitch_lags_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/find_pred_coefs_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/LPC_analysis_filter_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/LTP_analysis_filter_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/LTP_scale_ctrl_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/noise_shape_analysis_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/process_gains_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/regularize_correlations_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/residual_energy_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/warped_autocorrelation_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/wrappers_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/autocorrelation_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/burg_modified_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/bwexpander_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/energy_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/inner_product_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/k2a_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/LPC_inv_pred_gain_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/pitch_analysis_core_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/scale_copy_vector_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/scale_vector_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/schur_FLP.c \
  $(AR_SRC_ROOT)/opus/silk/float/sort_FLP.c \
  $(AR_SRC_ROOT)/opus/src/opus.c \
  $(AR_SRC_ROOT)/opus/src/opus_decoder.c \
  $(AR_SRC_ROOT)/opus/src/opus_encoder.c \
  $(AR_SRC_ROOT)/opus/src/opus_multistream.c \
  $(AR_SRC_ROOT)/opus/src/opus_multistream_encoder.c \
  $(AR_SRC_ROOT)/opus/src/opus_multistream_decoder.c \
  $(AR_SRC_ROOT)/opus/src/repacketizer.c \
  $(AR_SRC_ROOT)/opus/src/opus_projection_encoder.c \
  $(AR_SRC_ROOT)/opus/src/opus_projection_decoder.c \
  $(AR_SRC_ROOT)/opus/src/mapping_matrix.c \
  $(AR_SRC_ROOT)/opus/src/analysis.c \
  $(AR_SRC_ROOT)/opus/src/mlp.c \
  $(AR_SRC_ROOT)/opus/src/mlp_data.c

include $(BUILD_STATIC_LIBRARY)

#------------------------------
#- CORE FLAC (static library) -
#------------------------------

#./configure --disable-largefile --disable-asm-optimizations --disable-sse --disable-altivec --disable-vsx --disable-avx --disable-doxygen-docs --disable-xmms-plugin --disable-cpplibs --disable-ogg --disable-oggtest --disable-examples --disable-rpath

#xcode
#-DCPU_IS_BIG_ENDIAN=0 -DCPU_IS_LITTLE_ENDIAN=1 -DENABLE_64_BIT_WORDS=0 -DFLAC__ALIGN_MALLOC_DATA=1 -DFLAC__CPU_X86_64=1 -DFLAC__HAS_OGG=0 -DFLAC__HAS_X86INTRIN=1 -DFLAC__NO_ASM=1 -DFLAC__SYS_DARWIN=1 -DHAVE_BSWAP16=1 -DHAVE_BSWAP32=1 -DHAVE_CXX_VARARRAYS=1 -DHAVE_C_VARARRAYS=1 -DHAVE_DLFCN_H=1 -DHAVE_FSEEKO=1 -DHAVE_GETOPT_LONG=1 -DHAVE_ICONV=1 -DHAVE_INTTYPES_H=1 -DHAVE_LANGINFO_CODESET=1 -DHAVE_LROUND=1 -DHAVE_MEMORY_H=1 -DHAVE_STDINT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRINGS_H=1 -DHAVE_STRING_H=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_PARAM_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_TERMIOS_H=1 -DHAVE_TYPEOF=1 -DHAVE_UNISTD_H=1 -DHAVE_X86INTRIN_H=1 -DICONV_CONST -DLT_OBJDIR="\".libs/\"" -DPACKAGE="\"flac\"" -DPACKAGE_BUGREPORT="\"flac-dev@xiph.org\"" -DPACKAGE_NAME="\"flac\"" -DPACKAGE_STRING="\"flac 1.3.3\"" -DPACKAGE_TARNAME="\"flac\"" -DPACKAGE_URL="\"https://www.xiph.org/flac/\"" -DPACKAGE_VERSION="\"1.3.3\"" -DSIZEOF_OFF_T=8 -DSIZEOF_VOIDP=8 -DSTDC_HEADERS=1 -I$(PROJECT_DIR)/../../../src/flac -I$(PROJECT_DIR)/../../../src/flac/include -I$(PROJECT_DIR)/../../../src/flac/src/libFLAC/include

include $(CLEAR_VARS)

LOCAL_MODULE		:= osaudiorecorder-core-flac

LOCAL_C_INCLUDES	:=
LOCAL_SRC_FILES		:= 

LOCAL_CFLAGS		+= -DCPU_IS_BIG_ENDIAN=0 -DCPU_IS_LITTLE_ENDIAN=1 -DENABLE_64_BIT_WORDS=0 -DFLAC__ALIGN_MALLOC_DATA=1 -DFLAC__CPU_X86_64=1 -DFLAC__HAS_OGG=0 -DFLAC__HAS_X86INTRIN=1 -DFLAC__NO_ASM=1 -DFLAC__SYS_DARWIN=1 -DHAVE_BSWAP16=1 -DHAVE_BSWAP32=1 -DHAVE_CXX_VARARRAYS=1 -DHAVE_C_VARARRAYS=1 -DHAVE_DLFCN_H=1 -DHAVE_FSEEKO=1 -DHAVE_GETOPT_LONG=1 -DHAVE_ICONV=1 -DHAVE_INTTYPES_H=1 -DHAVE_LANGINFO_CODESET=1 -DHAVE_LROUND=1 -DHAVE_MEMORY_H=1 -DHAVE_STDINT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRINGS_H=1 -DHAVE_STRING_H=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_PARAM_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_TERMIOS_H=1 -DHAVE_TYPEOF=1 -DHAVE_UNISTD_H=1 -DHAVE_X86INTRIN_H=1 -DICONV_CONST -DLT_OBJDIR="\".libs/\"" -DPACKAGE="\"flac\"" -DPACKAGE_BUGREPORT="\"flac-dev@xiph.org\"" -DPACKAGE_NAME="\"flac\"" -DPACKAGE_STRING="\"flac 1.3.3\"" -DPACKAGE_TARNAME="\"flac\"" -DPACKAGE_URL="\"https://www.xiph.org/flac/\"" -DPACKAGE_VERSION="\"1.3.3\"" -DSIZEOF_OFF_T=8 -DSIZEOF_VOIDP=8 -DSTDC_HEADERS=1

LOCAL_C_INCLUDES	+= \
  $(AR_INC_ROOT)/flac \
  $(AR_INC_ROOT)/flac/include \
  $(AR_INC_ROOT)/flac/src/libFLAC/include

#armeabi-v7a, arm64-v8a, x86, x86_64

ifneq (,$(findstring arm,$(TARGET_ARCH_ABI)))
  #ARM specifics
  LOCAL_C_INCLUDES	+= 
  LOCAL_SRC_FILES	+= \
    
  LOCAL_SRC_FILES	+= \
    
else ifneq (,$(findstring x86,$(TARGET_ARCH_ABI)))
  #x86 specifics
  LOCAL_C_INCLUDES	+= 
  LOCAL_SRC_FILES	+= \
    
  LOCAL_SRC_FILES	+= \
    
endif

LOCAL_SRC_FILES		+= \
  $(AR_SRC_ROOT)/flac/src/libFLAC/bitmath.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/bitreader.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/bitwriter.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/cpu.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/crc.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/fixed.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/fixed_intrin_sse2.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/fixed_intrin_ssse3.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/float.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/format.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/lpc.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/lpc_intrin_sse.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/lpc_intrin_sse2.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/lpc_intrin_sse41.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/lpc_intrin_avx2.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/lpc_intrin_vsx.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/md5.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/memory.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/metadata_iterators.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/metadata_object.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/stream_decoder.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/stream_encoder.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/stream_encoder_intrin_sse2.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/stream_encoder_intrin_ssse3.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/stream_encoder_intrin_avx2.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/stream_encoder_framing.c \
  $(AR_SRC_ROOT)/flac/src/libFLAC/window.c \
  $(AR_SRC_ROOT)/flac/src/share/getopt/getopt.c \
  $(AR_SRC_ROOT)/flac/src/share/getopt/getopt1.c \
  $(AR_SRC_ROOT)/flac/src/share/grabbag/alloc.c \
  $(AR_SRC_ROOT)/flac/src/share/grabbag/cuesheet.c \
  $(AR_SRC_ROOT)/flac/src/share/grabbag/file.c \
  $(AR_SRC_ROOT)/flac/src/share/grabbag/picture.c \
  $(AR_SRC_ROOT)/flac/src/share/grabbag/replaygain.c \
  $(AR_SRC_ROOT)/flac/src/share/grabbag/seektable.c \
  $(AR_SRC_ROOT)/flac/src/share/grabbag/snprintf.c \
  $(AR_SRC_ROOT)/flac/src/share/utf8/charset.c \
  $(AR_SRC_ROOT)/flac/src/share/utf8/iconvert.c \
  $(AR_SRC_ROOT)/flac/src/share/utf8/utf8.c \
  $(AR_SRC_ROOT)/flac/src/share/replaygain_analysis/replaygain_analysis.c \
  $(AR_SRC_ROOT)/flac/src/share/replaygain_synthesis/replaygain_synthesis.c \
  $(AR_SRC_ROOT)/flac/src/flac/analyze.c \
  $(AR_SRC_ROOT)/flac/src/flac/decode.c \
  $(AR_SRC_ROOT)/flac/src/flac/encode.c \
  $(AR_SRC_ROOT)/flac/src/flac/foreign_metadata.c \
  $(AR_SRC_ROOT)/flac/src/flac/main.c \
  $(AR_SRC_ROOT)/flac/src/flac/local_string_utils.c \
  $(AR_SRC_ROOT)/flac/src/flac/utils.c \
  $(AR_SRC_ROOT)/flac/src/flac/vorbiscomment.c

include $(BUILD_STATIC_LIBRARY)

#------------------------------
#- CORE LIBOGG (static library) -
#------------------------------

#xcode
#-I$(PROJECT_DIR)/../../../src/libogg/include/

include $(CLEAR_VARS)

LOCAL_MODULE		:= osaudiorecorder-core-libogg

LOCAL_C_INCLUDES	:=
LOCAL_SRC_FILES		:= 

LOCAL_CFLAGS		+= 

LOCAL_C_INCLUDES	+= \
  $(AR_INC_ROOT)/libogg/include

#armeabi-v7a, arm64-v8a, x86, x86_64

ifneq (,$(findstring arm,$(TARGET_ARCH_ABI)))
  #ARM specifics
  LOCAL_C_INCLUDES	+= 
  LOCAL_SRC_FILES	+= \
    
  LOCAL_SRC_FILES	+= \
    
else ifneq (,$(findstring x86,$(TARGET_ARCH_ABI)))
  #x86 specifics
  LOCAL_C_INCLUDES	+= 
  LOCAL_SRC_FILES	+= \
    
  LOCAL_SRC_FILES	+= \
    
endif

LOCAL_SRC_FILES		+= \
  $(AR_SRC_ROOT)/libogg/src/framing.c \
  $(AR_SRC_ROOT)/libogg/src/bitwise.c

include $(BUILD_STATIC_LIBRARY)

#-------------------------
#- CORE (shared library) -
#-------------------------

include $(CLEAR_VARS)

LOCAL_MODULE		:= osaudiorecorder-core

LOCAL_STATIC_LIBRARIES	:= osaudiorecorder-core-flac osaudiorecorder-core-opus osaudiorecorder-core-libogg

LOCAL_LDLIBS		:= -llog -lOpenSLES -landroid

LOCAL_C_INCLUDES	+= \
  $(AR_INC_ROOT)/libogg/include \
  $(AR_INC_ROOT)/nixtla-audio

LOCAL_SRC_FILES		+= \
  $(AR_SRC_ROOT)/nixtla-audio/nixtla-audio.c \
  $(AR_SRC_ROOT)/osaudiorecorder.c \
  org_ostteam_audiorecorder_Core.c

include $(BUILD_SHARED_LIBRARY)
