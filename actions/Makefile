
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

include ../craydefs.mk
include ../cflags.mk
include ../codedirs.mk

ifndef ARFL
  ARFL=-rv
endif


COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(DYN) $(COMP)

$(warning DEFS in actions is $(DEFS))


ARFL=rv

ifneq ("X$(SHORT)", "X")
    POST0=cp lActions.a ../actions/; echo "copied"
else
    POST0=echo "ok"
endif

ACTSRC=$(wildcard *.cpp)
ACT=$(ACTSRC:.cpp=.o)

all: actions
	$(POST0)

lib: libActions.a
	$(POST0)


actions: libActions.a

libActions.a: $(ACT)
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libActions.a *.ti


%.o: %.cpp %.h 
	$(CC) $(COPT)  $(DEFS) -I${CORE} -I$(ICOSA) -I$(IO) -I$(GENES) -I$(UTILS_QHG)

