include $(REP_DIR)/lib/mk/fdk-aac.inc

LIBS += libm fdk-aac_sbrdec fdk-aac_sbrenc

AACDEC_SRC = \
	aacdec_drc.cpp \
	aacdec_hcr_bit.cpp \
	aacdec_hcr.cpp \
	aacdec_hcrs.cpp \
	aacdecoder.cpp \
	aacdecoder_lib.cpp \
	aacdec_pns.cpp \
	aacdec_tns.cpp \
	aac_ram.cpp \
	aac_rom.cpp \
	block.cpp \
	channel.cpp \
	channelinfo.cpp \
	conceal.cpp \
	ldfiltbank.cpp \
	pulsedata.cpp \
	rvlcbit.cpp \
	rvlcconceal.cpp \
	rvlc.cpp \
	stereo.cpp \

AACENC_SRC = \
	aacenc.cpp \
	aacenc_lib.cpp \
	aacenc_pns.cpp \
	aacEnc_ram.cpp \
	aacEnc_rom.cpp \
	aacenc_tns.cpp \
	adj_thr.cpp \
	band_nrg.cpp \
	bandwidth.cpp \
	bit_cnt.cpp \
	bitenc.cpp \
	block_switch.cpp \
	channel_map.cpp \
	chaosmeasure.cpp \
	dyn_bits.cpp \
	grp_data.cpp \
	intensity.cpp \
	line_pe.cpp \
	metadata_compressor.cpp \
	metadata_main.cpp \
	ms_stereo.cpp \
	noisedet.cpp \
	pnsparam.cpp \
	pre_echo_control.cpp \
	psy_configuration.cpp \
	psy_main.cpp \
	qc_main.cpp \
	quantize.cpp \
	sf_estim.cpp \
	spreading.cpp \
	tonality.cpp \
	transform.cpp \

FDK_SRC = \
	autocorr2nd.cpp \
	dct.cpp \
	FDK_bitbuffer.cpp \
	FDK_core.cpp \
	FDK_crc.cpp \
	FDK_hybrid.cpp \
	FDK_tools_rom.cpp \
	FDK_trigFcts.cpp \
	fft.cpp \
	fft_rad2.cpp \
	fixpoint_math.cpp \
	mdct.cpp \
	qmf.cpp \
	scale.cpp \

MPEGTPDEC_SRC = \
	tpdec_adif.cpp \
	tpdec_adts.cpp \
	tpdec_asc.cpp \
	tpdec_drm.cpp \
	tpdec_latm.cpp \
	tpdec_lib.cpp \

MPEGTPENC_SRC = \
	tpenc_adif.cpp \
	tpenc_adts.cpp \
	tpenc_asc.cpp \
	tpenc_latm.cpp \
	tpenc_lib.cpp \

PCMUTILS_SRC = \
	limiter.cpp \
	pcmutils_lib.cpp \

SYS_SRC = \
	cmdl_parser.cpp \
	conv_string.cpp \
	genericStds.cpp \
	wav_file.cpp \

SRC_CC := \
	$(AACDEC_SRC) \
	$(AACENC_SRC) \
	$(FDK_SRC) \
	$(MPEGTPDEC_SRC) \
	$(MPEGTPENC_SRC) \
	$(PCMUTILS_SRC) \
	$(SYS_SRC) \

vpath %.cpp $(FDK_AAC_SRC_DIR)/libAACdec/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libAACenc/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libFDK/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libMpegTPDec/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libMpegTPEnc/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libPCMutils/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libSBRdec/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libSBRenc/src
vpath %.cpp $(FDK_AAC_SRC_DIR)/libSYS/src

SHARED_LIB = yes
