SOPLEX_DIR = $(call select_from_ports,soplex)/src/lib/soplex/src/
LIBS    += libc zlib gmp stdcxx libm
INC_DIR += $(SOPLEX_DIR)
SRC_CC   = 	changesoplex.cpp \
			clufactor.cpp \
			didxset.cpp \
			enter.cpp \
			gzstream.cpp \
			idxset.cpp \
			leave.cpp \
			mpsinput.cpp \
			nameset.cpp \
			rational.cpp \
			slufactor.cpp \
			solverational.cpp \
			solvereal.cpp \
			soplex.cpp \
			soplexlegacy.cpp \
			spxautopr.cpp \
			spxbasis.cpp \
			spxboundflippingrt.cpp \
			spxbounds.cpp \
			spxchangebasis.cpp \
			spxdantzigpr.cpp \
			spxdefaultrt.cpp \
			spxdefines.cpp \
			spxdesc.cpp \
			spxdevexpr.cpp \
			spxequilisc.cpp \
			spxfastrt.cpp \
			spxfileio.cpp \
			spxgeometsc.cpp \
			spxgithash.cpp \
			spxharrisrt.cpp \
			spxhybridpr.cpp \
			spxid.cpp \
			spxlpbase_rational.cpp \
			spxlpbase_real.cpp \
			spxmainsm.cpp \
			spxout.cpp \
			spxparmultpr.cpp \
			spxquality.cpp \
			spxscaler.cpp \
			spxshift.cpp \
			spxsolve.cpp \
			spxsolver.cpp \
			spxstarter.cpp \
			spxsteeppr.cpp \
			spxsumst.cpp \
			spxvecs.cpp \
			spxvectorst.cpp \
			spxweightpr.cpp \
			spxweightst.cpp \
			spxwritestate.cpp \
			statistics.cpp \
			timer.cpp \
			unitvector.cpp \
			updatevector.cpp

CC_WARN  =

vpath %.cpp $(SOPLEX_DIR)

SHARED_LIB = yes
