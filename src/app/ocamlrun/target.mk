TARGET = ocamlrun
LIBS += ocaml-unix ocaml-str ocaml-threads posix libc

OCAML_PORT_DIR := $(call select_from_ports,ocaml)

BYTERUN_DIR := $(OCAML_PORT_DIR)/src/ocaml/byterun

INC_DIR += $(PRG_DIR)
INC_DIR += $(BYTERUN_DIR)

OCAML_VERSION_STRING := $(shell head -n 1 $(BYTERUN_DIR)/../VERSION)

CC_DEF += -DOCAML_VERSION_STRING=\"$(OCAML_VERSION_STRING)\"
CC_DEF += -DOCAML_VERSION=\"$(OCAML_VERSION_STRING)\"

# CC_DEF += -DDEBUG

PRIMS=\
  alloc.c array.c compare.c extern.c floats.c gc_ctrl.c hash.c \
  intern.c interp.c ints.c io.c lexing.c md5.c meta.c obj.c parsing.c \
  signals.c str.c sys.c callback.c weak.c finalise.c stacks.c \
  dynlink.c backtrace_prim.c backtrace.c spacetime.c afl.c \
  bigarray.c

BYTERUN_SRC=$(addsuffix .c, \
  interp misc stacks fix_code startup_aux startup \
  freelist major_gc minor_gc memory alloc roots globroots \
  fail signals signals_byt printexc backtrace_prim backtrace \
  compare ints floats str array io extern intern \
  hash sys meta parsing gc_ctrl md5 obj \
  lexing callback debugger weak compact finalise custom \
  dynlink spacetime afl unix bigarray main instrtrace)

primitives : $(addprefix $(BYTERUN_DIR)/,$(PRIMS))
	$(MSG_CONVERT $@)
	$(VERBOSE)sed -n -e "s/CAMLprim value \([a-z0-9_][a-z0-9_]*\).*/\1/p" $^ \
	  | LC_ALL=C sort | uniq > primitives

.headers: caml/opnames.h caml/jumptbl.h
	$(VERBOSE)touch $@

prims.c : primitives .headers
	$(MSG_CONVERT $@)
	$(VERBOSE)(echo '#define CAML_INTERNALS'; \
         echo '#include "caml/mlvalues.h"'; \
	 echo '#include "caml/prims.h"'; \
	 sed -e 's/.*/extern value &();/' primitives; \
	 echo 'c_primitive caml_builtin_cprim[] = {'; \
	 sed -e 's/.*/	&,/' primitives; \
	 echo '	 0 };'; \
	 echo 'char * caml_names_of_builtin_cprim[] = {'; \
	 sed -e 's/.*/	"&",/' primitives; \
	 echo '	 0 };') > prims.c

caml/opnames.h : $(BYTERUN_DIR)/caml/instruct.h
	$(MSG_CONVERT $@)
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)cat $^ | tr -d '\r' | \
	sed -e '/\/\*/d' \
	    -e '/^#/d' \
	    -e 's/enum /char * names_of_/' \
	    -e 's/{$$/[] = {/' \
	    -e 's/\([[:upper:]][[:upper:]_0-9]*\)/"\1"/g' > $@

caml/jumptbl.h: $(BYTERUN_DIR)/caml/instruct.h
	$(MSG_CONVERT $@)
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)sed -E -n -e '/^  /s/ ([A-Z])/ \&\&lbl_\1/gp' -e '/^}/q' $< > $@

caml/version.h:
	$(MSG_CONVERT $@)
	$(VERBOSE)mkdir -p $(dir $@)
	$(VERBOSE)echo #define OCAML_VERSION $(OCAML_VERSION_STRING) > $@

SRC_C += $(PRIMS)
SRC_C += $(BYTERUN_SRC)
SRC_C += prims.c

vpath %.c $(BYTERUN_DIR)

$(BYTERUN_SRC:.c=.o): .headers
