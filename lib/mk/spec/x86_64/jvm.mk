CC_OPT  = -DINCLUDE_SUFFIX_CPU=_x86 -DAMD64 -DHOTSPOT_LIB_ARCH='"amd64"'

INC_DIR       = $(call select_from_ports,jdk)/src/app/jdk/hotspot/src/cpu/x86/vm
JDK_GENERATED = $(call select_from_ports,jdk_generated)/src/app/jdk

INC_DIR += $(HOTSPOT_BASE)/os_cpu/bsd_x86/vm

SRC_CONE = cpu/x86/vm/c1_CodeStubs_x86.cpp \
           cpu/x86/vm/c1_FpuStackSim_x86.cpp \
           cpu/x86/vm/c1_FrameMap_x86.cpp \
           cpu/x86/vm/c1_LinearScan_x86.cpp \
           cpu/x86/vm/c1_LIRAssembler_x86.cpp \
           cpu/x86/vm/c1_LIRGenerator_x86.cpp \
           cpu/x86/vm/c1_LIR_x86.cpp \
           cpu/x86/vm/c1_MacroAssembler_x86.cpp \
           cpu/x86/vm/c1_Runtime1_x86.cpp

SRC_CTWO = cpu/x86/vm/c2_init_x86.cpp \
           cpu/x86/vm/compiledIC_x86.cpp \
           cpu/x86/vm/macroAssembler_x86.cpp \
           cpu/x86/vm/sharedRuntime_x86_64.cpp

SRC_CC = cpu/x86/vm/abstractInterpreter_x86.cpp \
         cpu/x86/vm/assembler_x86.cpp \
         cpu/x86/vm/debug_x86.cpp \
         cpu/x86/vm/depChecker_x86.cpp \
         cpu/x86/vm/frame_x86.cpp \
         cpu/x86/vm/icache_x86.cpp \
         cpu/x86/vm/icBuffer_x86.cpp \
         cpu/x86/vm/interp_masm_x86.cpp \
         cpu/x86/vm/interpreterRT_x86_64.cpp \
         cpu/x86/vm/jniFastGetField_x86_64.cpp \
         cpu/x86/vm/jvmciCodeInstaller_x86.cpp \
         cpu/x86/vm/macroAssembler_x86_cos.cpp \
         cpu/x86/vm/macroAssembler_x86_exp.cpp \
         cpu/x86/vm/macroAssembler_x86_log10.cpp \
         cpu/x86/vm/macroAssembler_x86_log.cpp \
         cpu/x86/vm/macroAssembler_x86_pow.cpp \
         cpu/x86/vm/macroAssembler_x86_sha.cpp \
         cpu/x86/vm/macroAssembler_x86_sin.cpp \
         cpu/x86/vm/macroAssembler_x86_tan.cpp \
         cpu/x86/vm/metaspaceShared_x86_64.cpp \
         cpu/x86/vm/methodHandles_x86.cpp \
         cpu/x86/vm/nativeInst_x86.cpp \
         cpu/x86/vm/register_definitions_x86.cpp \
         cpu/x86/vm/registerMap_x86.cpp \
         cpu/x86/vm/register_x86.cpp \
         cpu/x86/vm/relocInfo_x86.cpp \
         cpu/x86/vm/runtime_x86_64.cpp \
         cpu/x86/vm/sharedRuntime_x86.cpp \
         cpu/x86/vm/stubGenerator_x86_64.cpp \
         cpu/x86/vm/stubRoutines_x86_64.cpp \
         cpu/x86/vm/stubRoutines_x86.cpp \
         cpu/x86/vm/templateInterpreterGenerator_x86_64.cpp \
         cpu/x86/vm/templateInterpreterGenerator_x86.cpp \
         cpu/x86/vm/templateTable_x86.cpp \
         cpu/x86/vm/vmreg_x86.cpp \
         cpu/x86/vm/vm_version_x86.cpp \
         cpu/x86/vm/vtableStubs_x86_64.cpp \
         os_cpu/bsd_x86/vm/assembler_bsd_x86.cpp \
         os_cpu/bsd_x86/vm/os_bsd_x86.cpp \
         os_cpu/bsd_x86/vm/thread_bsd_x86.cpp \
         os_cpu/bsd_x86/vm/vm_version_bsd_x86.cpp

SRC_S = os_cpu/bsd_x86/vm/bsd_x86_64.s

#
# Force preprocessing
#
CC_OPT_os_cpu/bsd_x86/vm/bsd_x86_64 = -x assembler-with-cpp
CC_OPT_share/vm/runtime/rtmLocking = -DINCLUDE_RTM_OPT

#
# XXX: generated source
#
SRC_GEN = ad_x86.cpp ad_x86_format.cpp ad_x86_clone.cpp ad_x86_gen.cpp \
          ad_x86_expand.cpp ad_x86_misc.cpp  ad_x86_peephole.cpp \
          ad_x86_pipeline.cpp dfa_x86.cpp jvmtiEnterTrace.cpp jvmtiEnter.cpp

$(foreach FILE, $(SRC_GEN:.cpp=), $(eval CC_OPT_$(FILE) = -DLINUX -D_GNU_SOURCE -DCOMPILER2))
SRC_CC += $(SRC_GEN)


vpath ad_x86.cpp $(JDK_GENERATED)/src
vpath ad_x86_format.cpp $(JDK_GENERATED)/src
vpath ad_x86_clone.cpp $(JDK_GENERATED)/src
vpath ad_x86_gen.cpp $(JDK_GENERATED)/src
vpath ad_x86_expand.cpp $(JDK_GENERATED)/src
vpath ad_x86_misc.cpp $(JDK_GENERATED)/src
vpath ad_x86_peephole.cpp $(JDK_GENERATED)/src
vpath ad_x86_pipeline.cpp $(JDK_GENERATED)/src
vpath dfa_x86.cpp $(JDK_GENERATED)/src
vpath jvmtiEnterTrace.cpp $(JDK_GENERATED)/src
vpath jvmtiEnter.cpp $(JDK_GENERATED)/src


include $(REP_DIR)/lib/mk/jvm.inc
