LIBS         = stdcxx jzip jimage nio jnet libc verify
SHARED_LIB   = yes
HOTSPOT_BASE = $(call select_from_ports,jdk)/src/app/jdk/hotspot/src

CC_CXX_WARN_STRICT =

CC_CXX_OPT_STD = -std=gnu++17 -fconcepts

include $(REP_DIR)/lib/mk/jdk_version.inc

CC_OPT += -fpermissive -fno-delete-null-pointer-checks \
          -Wno-comment -Wno-reorder -Wno-switch -Wno-unused-variable \
          -Wno-delete-non-virtual-dtor -Wno-unused-function -Wno-format \
          -Wno-unused-local-typedefs -Wno-unused-but-set-variable \
          -Wno-parentheses -Wno-template-id-cdtor -Wno-nonnull

CC_OPT += -D__GENODE__

##XXX: may have to use our own
CC_OPT += -DINCLUDE_SUFFIX_OS=_bsd -DTARGET_COMPILER_gcc -D_ALLBSD_SOURCE -DUSE_LIBRARY_BASED_TLS_ONLY \
          -DVM_LITTLE_ENDIAN

CC_OPT += -DCOMPILER1 -DCOMPILER2

CC_OLEVEL = -O2

## DEBUGGING ##
#CC_OPT += -DASSERT

INC_DIR += $(HOTSPOT_BASE)/os/bsd/vm \
           $(HOTSPOT_BASE)/os/posix/vm \


INC_DIR += $(REP_DIR)/src/app/jdk/lib/include \
           $(JDK_GENERATED)/include \
           $(HOTSPOT_BASE)/share/vm \
           $(HOTSPOT_BASE)/share/vm/precompiled \
           $(HOTSPOT_BASE)/share/vm/prims

SRC_CC += os_genode.cpp dummies.cpp


SRC_CONE += share/vm/c1/c1_Canonicalizer.cpp \
            share/vm/c1/c1_CFGPrinter.cpp \
            share/vm/c1/c1_Compilation.cpp \
            share/vm/c1/c1_Compiler.cpp \
            share/vm/c1/c1_Defs.cpp \
            share/vm/c1/c1_FrameMap.cpp \
            share/vm/c1/c1_globals.cpp \
            share/vm/c1/c1_GraphBuilder.cpp \
            share/vm/c1/c1_Instruction.cpp \
            share/vm/c1/c1_InstructionPrinter.cpp \
            share/vm/c1/c1_IR.cpp \
            share/vm/c1/c1_LinearScan.cpp \
            share/vm/c1/c1_LIRAssembler.cpp \
            share/vm/c1/c1_LIR.cpp \
            share/vm/c1/c1_LIRGenerator.cpp \
            share/vm/c1/c1_Optimizer.cpp \
            share/vm/c1/c1_RangeCheckElimination.cpp \
            share/vm/c1/c1_Runtime1.cpp \
            share/vm/c1/c1_ValueMap.cpp \
            share/vm/c1/c1_ValueSet.cpp \
            share/vm/c1/c1_ValueStack.cpp \
            share/vm/c1/c1_ValueType.cpp \
            share/vm/ci/ciMethod.cpp

SRC_CTWO += share/vm/ci/bcEscapeAnalyzer.cpp \
            share/vm/ci/ciTypeFlow.cpp \
            share/vm/code/codeBlob.cpp \
            share/vm/jvmci/jvmci_globals.cpp \
            share/vm/opto/addnode.cpp \
            share/vm/opto/arraycopynode.cpp \
            share/vm/opto/block.cpp \
            share/vm/opto/buildOopMap.cpp \
            share/vm/opto/bytecodeInfo.cpp \
            share/vm/opto/c2compiler.cpp \
            share/vm/opto/callGenerator.cpp \
            share/vm/opto/callnode.cpp \
            share/vm/opto/castnode.cpp \
            share/vm/opto/cfgnode.cpp \
            share/vm/opto/chaitin.cpp \
            share/vm/opto/classes.cpp \
            share/vm/opto/coalesce.cpp \
            share/vm/opto/compile.cpp \
            share/vm/opto/connode.cpp \
            share/vm/opto/convertnode.cpp \
            share/vm/opto/countbitsnode.cpp \
            share/vm/opto/divnode.cpp \
            share/vm/opto/doCall.cpp \
            share/vm/opto/domgraph.cpp \
            share/vm/opto/escape.cpp \
            share/vm/opto/gcm.cpp \
            share/vm/opto/generateOptoStub.cpp \
            share/vm/opto/graphKit.cpp \
            share/vm/opto/idealGraphPrinter.cpp \
            share/vm/opto/idealKit.cpp \
            share/vm/opto/ifg.cpp \
            share/vm/opto/ifnode.cpp \
            share/vm/opto/indexSet.cpp \
            share/vm/opto/intrinsicnode.cpp \
            share/vm/opto/lcm.cpp \
            share/vm/opto/library_call.cpp \
            share/vm/opto/live.cpp \
            share/vm/opto/locknode.cpp \
            share/vm/opto/loopnode.cpp \
            share/vm/opto/loopopts.cpp \
            share/vm/opto/loopPredicate.cpp \
            share/vm/opto/loopTransform.cpp \
            share/vm/opto/loopUnswitch.cpp \
            share/vm/opto/machnode.cpp \
            share/vm/opto/macroArrayCopy.cpp \
            share/vm/opto/macro.cpp \
            share/vm/opto/matcher.cpp \
            share/vm/opto/mathexactnode.cpp \
            share/vm/opto/memnode.cpp \
            share/vm/opto/movenode.cpp \
            share/vm/opto/mulnode.cpp \
            share/vm/opto/multnode.cpp \
            share/vm/opto/narrowptrnode.cpp \
            share/vm/opto/node.cpp \
            share/vm/opto/opaquenode.cpp \
            share/vm/opto/opcodes.cpp \
            share/vm/opto/output.cpp \
            share/vm/opto/parse1.cpp \
            share/vm/opto/parse2.cpp \
            share/vm/opto/parse3.cpp \
            share/vm/opto/parseHelper.cpp \
            share/vm/opto/phase.cpp \
            share/vm/opto/phaseX.cpp \
            share/vm/opto/postaloc.cpp \
            share/vm/opto/regalloc.cpp \
            share/vm/opto/regmask.cpp \
            share/vm/opto/reg_split.cpp \
            share/vm/opto/replacednodes.cpp \
            share/vm/opto/rootnode.cpp \
            share/vm/opto/runtime.cpp \
            share/vm/opto/split_if.cpp \
            share/vm/opto/stringopts.cpp \
            share/vm/opto/subnode.cpp \
            share/vm/opto/superword.cpp \
            share/vm/opto/type.cpp \
            share/vm/opto/vectornode.cpp \
            share/vm/prims/jvm.cpp \
            share/vm/runtime/globals.cpp \
          share/vm/runtime/vframe.cpp \
            share/vm/runtime/sharedRuntime.cpp

CC_OPT_share/vm/runtime/vm_version += $(JDK_VERSION) \
              -DHOTSPOT_VERSION_STRING='"9-internal+0-adhoc.genode.openjdk-jdk9-jdk9"'\
              -DDEBUG_LEVEL='"release"' -DHOTSPOT_VM_DISTRO='"OpenJDK"' \
              -DHOTSPOT_BUILD_USER='"genode"'

CC_OPT_cpu/arm/vm/vm_version_arm_32 += -DKUSER_HELPER_VERSION_ADDR=0x0

SRC_CC += $(SRC_CONE) $(SRC_CTWO) \
          os/bsd/dtrace/generateJvmOffsets.cpp \
          os/bsd/vm/attachListener_bsd.cpp \
          os/bsd/vm/decoder_machO.cpp \
          os/bsd/vm/jvm_bsd.cpp \
          os/bsd/vm/osThread_bsd.cpp \
          os/bsd/vm/perfMemory_bsd.cpp \
          os/bsd/vm/stubRoutines_bsd.cpp \
          os/bsd/vm/threadCritical_bsd.cpp \
          os/linux/vm/decoder_linux.cpp \
          os/posix/vm/os_posix.cpp \
          os/posix/vm/threadLocalStorage_posix.cpp \
          os/posix/vm/vmError_posix.cpp \
          share/vm/asm/assembler.cpp \
          share/vm/asm/codeBuffer.cpp \
          share/vm/asm/register.cpp \
          share/vm/ci/ciArray.cpp \
          share/vm/ci/ciArrayKlass.cpp \
          share/vm/ci/ciBaseObject.cpp \
          share/vm/ci/ciCallSite.cpp \
          share/vm/ci/ciConstant.cpp \
          share/vm/ci/ciConstantPoolCache.cpp \
          share/vm/ci/ciEnv.cpp \
          share/vm/ci/ciExceptionHandler.cpp \
          share/vm/ci/ciField.cpp \
          share/vm/ci/ciFlags.cpp \
          share/vm/ci/ciInstance.cpp \
          share/vm/ci/ciInstanceKlass.cpp \
          share/vm/ci/ciKlass.cpp \
          share/vm/ci/ciMemberName.cpp \
          share/vm/ci/ciMetadata.cpp \
          share/vm/ci/ciMethodBlocks.cpp \
          share/vm/ci/ciMethodData.cpp \
          share/vm/ci/ciMethodHandle.cpp \
          share/vm/ci/ciNullObject.cpp \
          share/vm/ci/ciObjArray.cpp \
          share/vm/ci/ciObjArrayKlass.cpp \
          share/vm/ci/ciObject.cpp \
          share/vm/ci/ciObjectFactory.cpp \
          share/vm/ci/ciReplay.cpp \
          share/vm/ci/ciSignature.cpp \
          share/vm/ci/ciStreams.cpp \
          share/vm/ci/ciSymbol.cpp \
          share/vm/ci/ciTypeArray.cpp \
          share/vm/ci/ciTypeArrayKlass.cpp \
          share/vm/ci/ciType.cpp \
          share/vm/ci/ciUtilities.cpp \
          share/vm/classfile/altHashing.cpp \
          share/vm/classfile/bytecodeAssembler.cpp \
          share/vm/classfile/classFileError.cpp \
          share/vm/classfile/classFileParser.cpp \
          share/vm/classfile/classFileStream.cpp \
          share/vm/classfile/classListParser.cpp \
          share/vm/classfile/classLoader.cpp \
          share/vm/classfile/classLoaderData.cpp \
          share/vm/classfile/classLoaderExt.cpp \
          share/vm/classfile/classLoaderStats.cpp \
          share/vm/classfile/compactHashtable.cpp \
          share/vm/classfile/defaultMethods.cpp \
          share/vm/classfile/dictionary.cpp \
          share/vm/classfile/javaAssertions.cpp \
          share/vm/classfile/javaClasses.cpp \
          share/vm/classfile/klassFactory.cpp \
          share/vm/classfile/loaderConstraints.cpp \
          share/vm/classfile/metadataOnStackMark.cpp \
          share/vm/classfile/moduleEntry.cpp \
          share/vm/classfile/modules.cpp \
          share/vm/classfile/packageEntry.cpp \
          share/vm/classfile/placeholders.cpp \
          share/vm/classfile/resolutionErrors.cpp \
          share/vm/classfile/sharedPathsMiscInfo.cpp \
          share/vm/classfile/stackMapFrame.cpp \
          share/vm/classfile/stackMapTable.cpp \
          share/vm/classfile/stringTable.cpp \
          share/vm/classfile/symbolTable.cpp \
          share/vm/classfile/systemDictionary.cpp \
          share/vm/classfile/verificationType.cpp \
          share/vm/classfile/verifier.cpp \
          share/vm/classfile/vmSymbols.cpp \
          share/vm/code/codeCache.cpp \
          share/vm/code/compiledIC.cpp \
          share/vm/code/compiledMethod.cpp \
          share/vm/code/compressedStream.cpp \
          share/vm/code/debugInfo.cpp \
          share/vm/code/debugInfoRec.cpp \
          share/vm/code/dependencies.cpp \
          share/vm/code/dependencyContext.cpp \
          share/vm/code/exceptionHandlerTable.cpp \
          share/vm/code/icBuffer.cpp \
          share/vm/code/location.cpp \
          share/vm/code/nmethod.cpp \
          share/vm/code/oopRecorder.cpp \
          share/vm/code/pcDesc.cpp \
          share/vm/code/relocInfo.cpp \
          share/vm/code/relocInfo_ext.cpp \
          share/vm/code/scopeDesc.cpp \
          share/vm/code/stubs.cpp \
          share/vm/code/vmreg.cpp \
          share/vm/code/vtableStubs.cpp \
          share/vm/compiler/abstractCompiler.cpp \
          share/vm/compiler/compileBroker.cpp \
          share/vm/compiler/compileLog.cpp \
          share/vm/compiler/compilerDefinitions.cpp \
          share/vm/compiler/compilerDirectives.cpp \
          share/vm/compiler/compilerOracle.cpp \
          share/vm/compiler/compileTask.cpp \
          share/vm/compiler/directivesParser.cpp \
          share/vm/compiler/disassembler.cpp \
          share/vm/compiler/methodLiveness.cpp \
          share/vm/compiler/methodMatcher.cpp \
          share/vm/compiler/oopMap.cpp \
          share/vm/gc/cms/adaptiveFreeList.cpp \
          share/vm/gc/cms/allocationStats.cpp \
          share/vm/gc/cms/cmsCollectorPolicy.cpp \
          share/vm/gc/cms/cmsLockVerifier.cpp \
          share/vm/gc/cms/cmsOopClosures.cpp \
          share/vm/gc/cms/compactibleFreeListSpace.cpp \
          share/vm/gc/cms/concurrentMarkSweepGeneration.cpp \
          share/vm/gc/cms/concurrentMarkSweepThread.cpp \
          share/vm/gc/cms/freeChunk.cpp \
          share/vm/gc/cms/gSpaceCounters.cpp \
          share/vm/gc/cms/parCardTableModRefBS.cpp \
          share/vm/gc/cms/parNewGeneration.cpp \
          share/vm/gc/cms/parOopClosures.cpp \
          share/vm/gc/cms/promotionInfo.cpp \
          share/vm/gc/cms/vmCMSOperations.cpp \
          share/vm/gc/cms/yieldingWorkgroup.cpp \
          share/vm/gc/g1/collectionSetChooser.cpp \
          share/vm/gc/g1/concurrentG1Refine.cpp \
          share/vm/gc/g1/concurrentG1RefineThread.cpp \
          share/vm/gc/g1/concurrentMarkThread.cpp \
          share/vm/gc/g1/dirtyCardQueue.cpp \
          share/vm/gc/g1/g1Allocator.cpp \
          share/vm/gc/g1/g1Allocator_ext.cpp \
          share/vm/gc/g1/g1AllocRegion.cpp \
          share/vm/gc/g1/g1Analytics.cpp \
          share/vm/gc/g1/g1BiasedArray.cpp \
          share/vm/gc/g1/g1BlockOffsetTable.cpp \
          share/vm/gc/g1/g1CardCounts.cpp \
          share/vm/gc/g1/g1CardLiveData.cpp \
          share/vm/gc/g1/g1CodeBlobClosure.cpp \
          share/vm/gc/g1/g1CodeCacheRemSet.cpp \
          share/vm/gc/g1/g1CollectedHeap.cpp \
          share/vm/gc/g1/g1CollectedHeap_ext.cpp \
          share/vm/gc/g1/g1CollectionSet.cpp \
          share/vm/gc/g1/g1CollectorPolicy.cpp \
          share/vm/gc/g1/g1ConcurrentMark.cpp \
          share/vm/gc/g1/g1ConcurrentMarkObjArrayProcessor.cpp \
          share/vm/gc/g1/g1DefaultPolicy.cpp \
          share/vm/gc/g1/g1EvacFailure.cpp \
          share/vm/gc/g1/g1EvacStats.cpp \
          share/vm/gc/g1/g1FromCardCache.cpp \
          share/vm/gc/g1/g1GCPhaseTimes.cpp \
          share/vm/gc/g1/g1_globals.cpp \
          share/vm/gc/g1/g1HeapSizingPolicy.cpp \
          share/vm/gc/g1/g1HeapSizingPolicy_ext.cpp \
          share/vm/gc/g1/g1HeapTransition.cpp \
          share/vm/gc/g1/g1HeapVerifier.cpp \
          share/vm/gc/g1/g1HotCardCache.cpp \
          share/vm/gc/g1/g1IHOPControl.cpp \
          share/vm/gc/g1/g1MarkSweep.cpp \
          share/vm/gc/g1/g1MarkSweep_ext.cpp \
          share/vm/gc/g1/g1MMUTracker.cpp \
          share/vm/gc/g1/g1MonitoringSupport.cpp \
          share/vm/gc/g1/g1OopClosures.cpp \
          share/vm/gc/g1/g1PageBasedVirtualSpace.cpp \
          share/vm/gc/g1/g1ParScanThreadState.cpp \
          share/vm/gc/g1/g1ParScanThreadState_ext.cpp \
          share/vm/gc/g1/g1RegionToSpaceMapper.cpp \
          share/vm/gc/g1/g1RemSet.cpp \
          share/vm/gc/g1/g1RemSetSummary.cpp \
          share/vm/gc/g1/g1RootClosures.cpp \
          share/vm/gc/g1/g1RootClosures_ext.cpp \
          share/vm/gc/g1/g1RootProcessor.cpp \
          share/vm/gc/g1/g1SATBCardTableModRefBS.cpp \
          share/vm/gc/g1/g1StringDedup.cpp \
          share/vm/gc/g1/g1StringDedupQueue.cpp \
          share/vm/gc/g1/g1StringDedupStat.cpp \
          share/vm/gc/g1/g1StringDedupTable.cpp \
          share/vm/gc/g1/g1StringDedupThread.cpp \
          share/vm/gc/g1/g1SurvivorRegions.cpp \
          share/vm/gc/g1/g1YoungGenSizer.cpp \
          share/vm/gc/g1/g1YoungRemSetSamplingThread.cpp \
          share/vm/gc/g1/heapRegion.cpp \
          share/vm/gc/g1/heapRegionManager.cpp \
          share/vm/gc/g1/heapRegionRemSet.cpp \
          share/vm/gc/g1/heapRegionSet.cpp \
          share/vm/gc/g1/heapRegionTracer.cpp \
          share/vm/gc/g1/heapRegionType.cpp \
          share/vm/gc/g1/hSpaceCounters.cpp \
          share/vm/gc/g1/ptrQueue.cpp \
          share/vm/gc/g1/satbMarkQueue.cpp \
          share/vm/gc/g1/sparsePRT.cpp \
          share/vm/gc/g1/survRateGroup.cpp \
          share/vm/gc/g1/suspendibleThreadSet.cpp \
          share/vm/gc/g1/vm_operations_g1.cpp \
          share/vm/gc/g1/workerDataArray.cpp \
          share/vm/gc/parallel/adjoiningGenerations.cpp \
          share/vm/gc/parallel/adjoiningVirtualSpaces.cpp \
          share/vm/gc/parallel/asPSOldGen.cpp \
          share/vm/gc/parallel/asPSYoungGen.cpp \
          share/vm/gc/parallel/cardTableExtension.cpp \
          share/vm/gc/parallel/gcAdaptivePolicyCounters.cpp \
          share/vm/gc/parallel/gcTaskManager.cpp \
          share/vm/gc/parallel/gcTaskThread.cpp \
          share/vm/gc/parallel/generationSizer.cpp \
          share/vm/gc/parallel/immutableSpace.cpp \
          share/vm/gc/parallel/mutableNUMASpace.cpp \
          share/vm/gc/parallel/mutableSpace.cpp \
          share/vm/gc/parallel/objectStartArray.cpp \
          share/vm/gc/parallel/parallelScavengeHeap.cpp \
          share/vm/gc/parallel/parMarkBitMap.cpp \
          share/vm/gc/parallel/pcTasks.cpp \
          share/vm/gc/parallel/psAdaptiveSizePolicy.cpp \
          share/vm/gc/parallel/psCompactionManager.cpp \
          share/vm/gc/parallel/psGCAdaptivePolicyCounters.cpp \
          share/vm/gc/parallel/psGenerationCounters.cpp \
          share/vm/gc/parallel/psMarkSweep.cpp \
          share/vm/gc/parallel/psMarkSweepDecorator.cpp \
          share/vm/gc/parallel/psOldGen.cpp \
          share/vm/gc/parallel/psParallelCompact.cpp \
          share/vm/gc/parallel/psPromotionLAB.cpp \
          share/vm/gc/parallel/psPromotionManager.cpp \
          share/vm/gc/parallel/psScavenge.cpp \
          share/vm/gc/parallel/psTasks.cpp \
          share/vm/gc/parallel/psVirtualspace.cpp \
          share/vm/gc/parallel/psYoungGen.cpp \
          share/vm/gc/parallel/spaceCounters.cpp \
          share/vm/gc/parallel/vmPSOperations.cpp \
          share/vm/gc/serial/cSpaceCounters.cpp \
          share/vm/gc/serial/defNewGeneration.cpp \
          share/vm/gc/serial/genMarkSweep.cpp \
          share/vm/gc/serial/markSweep.cpp \
          share/vm/gc/serial/tenuredGeneration.cpp \
          share/vm/gc/shared/adaptiveSizePolicy.cpp \
          share/vm/gc/shared/ageTable.cpp \
          share/vm/gc/shared/ageTableTracer.cpp \
          share/vm/gc/shared/allocTracer.cpp \
          share/vm/gc/shared/barrierSet.cpp \
          share/vm/gc/shared/blockOffsetTable.cpp \
          share/vm/gc/shared/cardGeneration.cpp \
          share/vm/gc/shared/cardTableModRefBS.cpp \
          share/vm/gc/shared/cardTableModRefBSForCTRS.cpp \
          share/vm/gc/shared/cardTableRS.cpp \
          share/vm/gc/shared/collectedHeap.cpp \
          share/vm/gc/shared/collectorCounters.cpp \
          share/vm/gc/shared/collectorPolicy.cpp \
          share/vm/gc/shared/concurrentGCThread.cpp \
          share/vm/gc/shared/gcCause.cpp \
          share/vm/gc/shared/gcId.cpp \
          share/vm/gc/shared/gcLocker.cpp \
          share/vm/gc/shared/gcPolicyCounters.cpp \
          share/vm/gc/shared/gcStats.cpp \
          share/vm/gc/shared/gcTimer.cpp \
          share/vm/gc/shared/gcTrace.cpp \
          share/vm/gc/shared/gcTraceSend.cpp \
          share/vm/gc/shared/gcTraceTime.cpp \
          share/vm/gc/shared/gcUtil.cpp \
          share/vm/gc/shared/genCollectedHeap.cpp \
          share/vm/gc/shared/generationCounters.cpp \
          share/vm/gc/shared/generation.cpp \
          share/vm/gc/shared/generationSpec.cpp \
          share/vm/gc/shared/genOopClosures.cpp \
          share/vm/gc/shared/objectCountEventSender.cpp \
          share/vm/gc/shared/plab.cpp \
          share/vm/gc/shared/preservedMarks.cpp \
          share/vm/gc/shared/referencePolicy.cpp \
          share/vm/gc/shared/referenceProcessor.cpp \
          share/vm/gc/shared/space.cpp \
          share/vm/gc/shared/spaceDecorator.cpp \
          share/vm/gc/shared/strongRootsScope.cpp \
          share/vm/gc/shared/taskqueue.cpp \
          share/vm/gc/shared/threadLocalAllocBuffer.cpp \
          share/vm/gc/shared/vmGCOperations.cpp \
          share/vm/gc/shared/workgroup.cpp \
          share/vm/interpreter/abstractInterpreter.cpp \
          share/vm/interpreter/bytecode.cpp \
          share/vm/interpreter/bytecodeHistogram.cpp \
          share/vm/interpreter/bytecodeInterpreter.cpp \
          share/vm/interpreter/bytecodes.cpp \
          share/vm/interpreter/bytecodeStream.cpp \
          share/vm/interpreter/bytecodeTracer.cpp \
          share/vm/interpreter/cppInterpreter.cpp \
          share/vm/interpreter/cppInterpreterGenerator.cpp \
          share/vm/interpreter/interpreter.cpp \
          share/vm/interpreter/interpreterRuntime.cpp \
          share/vm/interpreter/invocationCounter.cpp \
          share/vm/interpreter/linkResolver.cpp \
          share/vm/interpreter/oopMapCache.cpp \
          share/vm/interpreter/rewriter.cpp \
          share/vm/interpreter/templateInterpreter.cpp \
          share/vm/interpreter/templateInterpreterGenerator.cpp \
          share/vm/interpreter/templateTable.cpp \
          share/vm/jvmci/jvmciCodeInstaller.cpp \
          share/vm/jvmci/jvmciCompiler.cpp \
          share/vm/jvmci/jvmciCompilerToVM.cpp \
          share/vm/jvmci/jvmciEnv.cpp \
          share/vm/jvmci/jvmciJavaClasses.cpp \
          share/vm/jvmci/jvmciRuntime.cpp \
          share/vm/jvmci/vmStructs_jvmci.cpp \
          share/vm/libadt/dict.cpp \
          share/vm/libadt/set.cpp \
          share/vm/libadt/vectset.cpp \
          share/vm/logging/logConfiguration.cpp \
          share/vm/logging/logDecorations.cpp \
          share/vm/logging/logDecorators.cpp \
          share/vm/logging/logDiagnosticCommand.cpp \
          share/vm/logging/logFileOutput.cpp \
          share/vm/logging/logFileStreamOutput.cpp \
          share/vm/logging/logLevel.cpp \
          share/vm/logging/logMessageBuffer.cpp \
          share/vm/logging/logOutput.cpp \
          share/vm/logging/logOutputList.cpp \
          share/vm/logging/logStream.cpp \
          share/vm/logging/logTag.cpp \
          share/vm/logging/logTagLevelExpression.cpp \
          share/vm/logging/logTagSet.cpp \
          share/vm/logging/logTagSetDescriptions.cpp \
          share/vm/memory/allocation.cpp \
          share/vm/memory/binaryTreeDictionary.cpp \
          share/vm/memory/filemap.cpp \
          share/vm/memory/freeBlockDictionary.cpp \
          share/vm/memory/freeList.cpp \
          share/vm/memory/guardedMemory.cpp \
          share/vm/memory/heap.cpp \
          share/vm/memory/heapInspection.cpp \
          share/vm/memory/iterator.cpp \
          share/vm/memory/memRegion.cpp \
          share/vm/memory/metachunk.cpp \
          share/vm/memory/metaspaceCounters.cpp \
          share/vm/memory/metaspace.cpp \
          share/vm/memory/metaspaceShared.cpp \
          share/vm/memory/metaspaceTracer.cpp \
          share/vm/memory/oopFactory.cpp \
          share/vm/memory/resourceArea.cpp \
          share/vm/memory/universe.cpp \
          share/vm/memory/universe_ext.cpp \
          share/vm/memory/virtualspace.cpp \
          share/vm/oops/annotations.cpp \
          share/vm/oops/arrayKlass.cpp \
          share/vm/oops/compiledICHolder.cpp \
          share/vm/oops/constantPool.cpp \
          share/vm/oops/constMethod.cpp \
          share/vm/oops/cpCache.cpp \
          share/vm/oops/generateOopMap.cpp \
          share/vm/oops/instanceKlass.cpp \
          share/vm/oops/instanceMirrorKlass.cpp \
          share/vm/oops/instanceOop.cpp \
          share/vm/oops/instanceRefKlass.cpp \
          share/vm/oops/klass.cpp \
          share/vm/oops/klassVtable.cpp \
          share/vm/oops/markOop.cpp \
          share/vm/oops/metadata.cpp \
          share/vm/oops/methodCounters.cpp \
          share/vm/oops/method.cpp \
          share/vm/oops/methodData.cpp \
          share/vm/oops/objArrayKlass.cpp \
          share/vm/oops/objArrayOop.cpp \
          share/vm/oops/oop.cpp \
          share/vm/oops/oopsHierarchy.cpp \
          share/vm/oops/symbol.cpp \
          share/vm/oops/typeArrayKlass.cpp \
          share/vm/opto/c2_globals.cpp \
          share/vm/prims/evmCompat.cpp \
          share/vm/prims/forte.cpp \
          share/vm/prims/jniCheck.cpp \
          share/vm/prims/jni.cpp \
          share/vm/prims/jniFastGetField.cpp \
          share/vm/prims/jvmtiClassFileReconstituter.cpp \
          share/vm/prims/jvmtiCodeBlobEvents.cpp \
          share/vm/prims/jvmtiEnvBase.cpp \
          share/vm/prims/jvmtiEnv.cpp \
          share/vm/prims/jvmtiEnvThreadState.cpp \
          share/vm/prims/jvmtiEventController.cpp \
          share/vm/prims/jvmtiExport.cpp \
          share/vm/prims/jvmtiExtensions.cpp \
          share/vm/prims/jvmtiGetLoadedClasses.cpp \
          share/vm/prims/jvmtiImpl.cpp \
          share/vm/prims/jvmtiManageCapabilities.cpp \
          share/vm/prims/jvmtiRawMonitor.cpp \
          share/vm/prims/jvmtiRedefineClasses.cpp \
          share/vm/prims/jvmtiTagMap.cpp \
          share/vm/prims/jvmtiThreadState.cpp \
          share/vm/prims/jvmtiTrace.cpp \
          share/vm/prims/jvmtiUtil.cpp \
          share/vm/prims/methodComparator.cpp \
          share/vm/prims/methodHandles.cpp \
          share/vm/prims/nativeLookup.cpp \
          share/vm/prims/perf.cpp \
          share/vm/prims/privilegedStack.cpp \
          share/vm/prims/stackwalk.cpp \
          share/vm/prims/unsafe.cpp \
          share/vm/prims/wbtestmethods/parserTests.cpp \
          share/vm/prims/whitebox.cpp \
          share/vm/prims/whitebox_ext.cpp \
          share/vm/runtime/advancedThresholdPolicy.cpp \
          share/vm/runtime/arguments.cpp \
          share/vm/runtime/arguments_ext.cpp \
          share/vm/runtime/basicLock.cpp \
          share/vm/runtime/biasedLocking.cpp \
          share/vm/runtime/commandLineFlagConstraintList.cpp \
          share/vm/runtime/commandLineFlagConstraintsCompiler.cpp \
          share/vm/runtime/commandLineFlagConstraintsGC.cpp \
          share/vm/runtime/commandLineFlagConstraintsRuntime.cpp \
          share/vm/runtime/commandLineFlagRangeList.cpp \
          share/vm/runtime/commandLineFlagWriteableList.cpp \
          share/vm/runtime/compilationPolicy.cpp \
          share/vm/runtime/deoptimization.cpp \
          share/vm/runtime/fieldDescriptor.cpp \
          share/vm/runtime/fieldType.cpp \
          share/vm/runtime/fprofiler.cpp \
          share/vm/runtime/frame.cpp \
          share/vm/runtime/handles.cpp \
          share/vm/runtime/icache.cpp \
          share/vm/runtime/init.cpp \
          share/vm/runtime/interfaceSupport.cpp \
          share/vm/runtime/javaCalls.cpp \
          share/vm/runtime/java.cpp \
          share/vm/runtime/jniHandles.cpp \
          share/vm/runtime/jniPeriodicChecker.cpp \
          share/vm/runtime/memprofiler.cpp \
          share/vm/runtime/monitorChunk.cpp \
          share/vm/runtime/mutex.cpp \
          share/vm/runtime/mutexLocker.cpp \
          share/vm/runtime/objectMonitor.cpp \
          share/vm/runtime/orderAccess.cpp \
          share/vm/runtime/os.cpp \
          share/vm/runtime/osThread.cpp \
          share/vm/runtime/park.cpp \
          share/vm/runtime/perfData.cpp \
          share/vm/runtime/perfMemory.cpp \
          share/vm/runtime/reflection.cpp \
          share/vm/runtime/reflectionUtils.cpp \
          share/vm/runtime/relocator.cpp \
          share/vm/runtime/rframe.cpp \
          share/vm/runtime/rtmLocking.cpp \
          share/vm/runtime/safepoint.cpp \
          share/vm/runtime/serviceThread.cpp \
          share/vm/runtime/sharedRuntimeTrans.cpp \
          share/vm/runtime/sharedRuntimeTrig.cpp \
          share/vm/runtime/signature.cpp \
          share/vm/runtime/simpleThresholdPolicy.cpp \
          share/vm/runtime/stackValueCollection.cpp \
          share/vm/runtime/stackValue.cpp \
          share/vm/runtime/statSampler.cpp \
          share/vm/runtime/stubCodeGenerator.cpp \
          share/vm/runtime/stubRoutines.cpp \
          share/vm/runtime/sweeper.cpp \
          share/vm/runtime/synchronizer.cpp \
          share/vm/runtime/task.cpp \
          share/vm/runtime/thread.cpp \
          share/vm/runtime/thread_ext.cpp \
          share/vm/runtime/timer.cpp \
          share/vm/runtime/timerTrace.cpp \
          share/vm/runtime/unhandledOops.cpp \
          share/vm/runtime/vframeArray.cpp \
          share/vm/runtime/vframe_hp.cpp \
          share/vm/runtime/vm_operations.cpp \
          share/vm/runtime/vmStructs.cpp \
          share/vm/runtime/vmThread.cpp \
          share/vm/runtime/vm_version.cpp \
          share/vm/services/attachListener.cpp \
          share/vm/services/classLoadingService.cpp \
          share/vm/services/diagnosticArgument.cpp \
          share/vm/services/diagnosticCommand.cpp \
          share/vm/services/diagnosticFramework.cpp \
          share/vm/services/dtraceAttacher.cpp \
          share/vm/services/g1MemoryPool.cpp \
          share/vm/services/gcNotifier.cpp \
          share/vm/services/heapDumper.cpp \
          share/vm/services/lowMemoryDetector.cpp \
          share/vm/services/mallocSiteTable.cpp \
          share/vm/services/mallocTracker.cpp \
          share/vm/services/management.cpp \
          share/vm/services/memBaseline.cpp \
          share/vm/services/memoryManager.cpp \
          share/vm/services/memoryPool.cpp \
          share/vm/services/memoryService.cpp \
          share/vm/services/memReporter.cpp \
          share/vm/services/memTracker.cpp \
          share/vm/services/nmtCommon.cpp \
          share/vm/services/nmtDCmd.cpp \
          share/vm/services/psMemoryPool.cpp \
          share/vm/services/runtimeService.cpp \
          share/vm/services/threadService.cpp \
          share/vm/services/virtualMemoryTracker.cpp \
          share/vm/services/writeableFlags.cpp \
          share/vm/trace/traceBackend.cpp \
          share/vm/trace/traceStream.cpp \
          share/vm/utilities/accessFlags.cpp \
          share/vm/utilities/bitMap.cpp \
          share/vm/utilities/constantTag.cpp \
          share/vm/utilities/copy.cpp \
          share/vm/utilities/debug.cpp \
          share/vm/utilities/decoder.cpp \
          share/vm/utilities/decoder_elf.cpp \
          share/vm/utilities/elfFile.cpp \
          share/vm/utilities/elfFuncDescTable.cpp \
          share/vm/utilities/elfStringTable.cpp \
          share/vm/utilities/elfSymbolTable.cpp \
          share/vm/utilities/errorReporter.cpp \
          share/vm/utilities/events.cpp \
          share/vm/utilities/exceptions.cpp \
          share/vm/utilities/globalDefinitions.cpp \
          share/vm/utilities/growableArray.cpp \
          share/vm/utilities/hashtable.cpp \
          share/vm/utilities/histogram.cpp \
          share/vm/utilities/internalVMTests.cpp \
          share/vm/utilities/intHisto.cpp \
          share/vm/utilities/json.cpp \
          share/vm/utilities/nativeCallStack.cpp \
          share/vm/utilities/numberSeq.cpp \
          share/vm/utilities/ostream.cpp \
          share/vm/utilities/preserveException.cpp \
          share/vm/utilities/sizes.cpp \
          share/vm/utilities/stringUtils.cpp \
          share/vm/utilities/ticks.cpp \
          share/vm/utilities/utf8.cpp \
          share/vm/utilities/vmError.cpp \
          share/vm/utilities/xmlstream.cpp

CC_OPT_share/vm/utilities/vmError += -Wno-narrowing

# error for gnu++17 in base/include/base/memory.h
#
# in Genode::Memory::Constrained_obj_allocator
#
# using Result     = Allocation::Attempt;
#
# error: need ‘typename’ before
# ‘Genode::Memory::Constrained_obj_allocator< <template-parameter-1-1> >::Allocation::Attempt’
# because ‘Genode::Memory::Constrained_obj_allocator< <template-parameter-1-1> >::Allocation’
# is a dependent scope
#
# -> use gnu++20 for this file
CC_OPT_os_genode += -std=gnu++20

vpath os_genode.cpp $(REP_DIR)/src/app/jdk/lib/jvm
vpath dummies.cpp $(REP_DIR)/src/app/jdk/lib/jvm
vpath %.cpp $(HOTSPOT_BASE)
vpath %.s   $(HOTSPOT_BASE)

# vi: set ft=make :
