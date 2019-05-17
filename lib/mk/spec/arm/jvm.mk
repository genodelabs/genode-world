CC_OPT  = -DINCLUDE_SUFFIX_CPU=_arm -DHOTSPOT_LIB_ARCH='"arm"' -DARM -DARM32 \
          -Dlseek64=lseek

INC_DIR       = $(call select_from_ports,jdk)/src/app/jdk/hotspot/src/cpu/arm/vm
JDK_GENERATED = $(call select_from_ports,jdk_generated)/src/app/jdk

INC_DIR += $(HOTSPOT_BASE)/os_cpu/bsd_zero/vm \
           $(HOTSPOT_BASE)/os_cpu/linux_arm/vm

SRC_CC = os_genode_arm.cpp

SRC_CONE = cpu/arm/vm/c1_CodeStubs_arm.cpp \
           cpu/arm/vm/c1_FpuStackSim_arm.cpp \
           cpu/arm/vm/c1_FrameMap_arm.cpp \
           cpu/arm/vm/c1_LinearScan_arm.cpp \
           cpu/arm/vm/c1_LIRAssembler_arm.cpp \
           cpu/arm/vm/c1_LIRGenerator_arm.cpp \
           cpu/arm/vm/c1_LIR_arm.cpp \
           cpu/arm/vm/c1_MacroAssembler_arm.cpp \
           cpu/arm/vm/c1_Runtime1_arm.cpp

SRC_CTWO = cpu/arm/vm/assembler_arm_32.cpp \
           cpu/arm/vm/compiledIC_arm.cpp \
           cpu/arm/vm/macroAssembler_arm.cpp \
           cpu/arm/vm/sharedRuntime_arm.cpp

SRC_CC += cpu/arm/vm/abstractInterpreter_arm.cpp \
          cpu/arm/vm/assembler_arm.cpp \
          cpu/arm/vm/debug_arm.cpp \
          cpu/arm/vm/depChecker_arm.cpp \
          cpu/arm/vm/frame_arm.cpp \
          cpu/arm/vm/icache_arm.cpp \
          cpu/arm/vm/icBuffer_arm.cpp \
          cpu/arm/vm/interp_masm_arm.cpp \
          cpu/arm/vm/interpreterRT_arm.cpp \
          cpu/arm/vm/jniFastGetField_arm.cpp \
          cpu/arm/vm/jvmciCodeInstaller_arm.cpp \
          cpu/arm/vm/macroAssembler_arm.cpp \
          cpu/arm/vm/metaspaceShared_arm.cpp \
          cpu/arm/vm/methodHandles_arm.cpp \
          cpu/arm/vm/nativeInst_arm_32.cpp \
          cpu/arm/vm/register_definitions_arm.cpp \
          cpu/arm/vm/register_arm.cpp \
          cpu/arm/vm/relocInfo_arm.cpp \
          cpu/arm/vm/runtime_arm.cpp \
          cpu/arm/vm/sharedRuntime_arm.cpp \
          cpu/arm/vm/stubGenerator_arm.cpp \
          cpu/arm/vm/stubRoutines_arm.cpp \
          cpu/arm/vm/templateInterpreterGenerator_arm.cpp \
          cpu/arm/vm/templateInterpreterGenerator_arm.cpp \
          cpu/arm/vm/templateTable_arm.cpp \
          cpu/arm/vm/vmreg_arm.cpp \
          cpu/arm/vm/vm_version_arm_32.cpp \
          cpu/arm/vm/vtableStubs_arm.cpp \
          os_cpu/linux_arm/vm/macroAssembler_linux_arm_32.cpp

SRC_S = os_cpu/linux_arm/vm/linux_arm_32.s
##
## Force preprocessing
##
CC_OPT_cpu/arm/vm/runtime_arm += -DCOMPILER2

#
# XXX: generated source
#
SRC_GEN = ad_arm.cpp ad_arm_format.cpp ad_arm_clone.cpp ad_arm_gen.cpp \
          ad_arm_expand.cpp ad_arm_misc.cpp  ad_arm_peephole.cpp \
          ad_arm_pipeline.cpp dfa_arm.cpp jvmtiEnterTrace.cpp jvmtiEnter.cpp

$(foreach FILE, $(SRC_GEN:.cpp=), $(eval CC_OPT_$(FILE) = -DLINUX -D_GNU_SOURCE -DCOMPILER2))
SRC_CC += $(SRC_GEN)


vpath ad_arm.cpp $(JDK_GENERATED)/src
vpath ad_arm_format.cpp $(JDK_GENERATED)/src
vpath ad_arm_clone.cpp $(JDK_GENERATED)/src
vpath ad_arm_gen.cpp $(JDK_GENERATED)/src
vpath ad_arm_expand.cpp $(JDK_GENERATED)/src
vpath ad_arm_misc.cpp $(JDK_GENERATED)/src
vpath ad_arm_peephole.cpp $(JDK_GENERATED)/src
vpath ad_arm_pipeline.cpp $(JDK_GENERATED)/src
vpath dfa_arm.cpp $(JDK_GENERATED)/src
vpath jvmtiEnterTrace.cpp $(JDK_GENERATED)/src
vpath jvmtiEnter.cpp $(JDK_GENERATED)/src
vpath os_genode_arm.cpp $(REP_DIR)/src/app/jdk/lib/jvm/spec/arm

include $(REP_DIR)/lib/mk/jvm.inc
