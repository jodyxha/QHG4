
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifndef QHG4_DIR
  QHG4_DIR=..
endif
include ${QHG4_DIR}/mk_includes/craydefs.mk
include ${QHG4_DIR}/mk_includes/cflags.mk
include ${QHG4_DIR}/mk_includes/codedirs.mk

ifndef ARFL
  ARFL=-rv
endif


COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(DYN) $(COMP)

ARFL=rv

ifneq ("X$(SHORT)", "X")
    ifneq  ("X$(DYN_POP)", "X")
        POST0=cp *.o libPopulations.a ../populations/; echo "copied"
    else
        POST0=cp libPopulations.a ../populations/; echo "copied"
    endif
else
    POST0=echo "ok"
endif

ifneq  ("X$(DYN_POP)", "X")
	ARCH=
else
	ARCH=$(POPS)
endif

POPSRC=$(wildcard *Pop.cpp)
POPS=$(POPSRC:.cpp=.o)

all: populations
	$(POST0)

CONFIGURE:
	$(POPULATIONS)/configure.sh

lib: CONFIGURE libPopulations.a
	$(POST0)

populations: CONFIGURE libPopulations.a


StatPopFactory.o: StatPopFactory.cpp StatPopFactory.h PopulationFactory.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA) -I$(IO) -I$(ACTIONS) -I$(CORE) -I$(GENES) 

DynPopFactory.o: DynPopFactory.cpp DynPopFactory.h PopulationFactory.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA) -I$(IO) -I$(ACTIONS) -I$(CORE) -I$(GENES) 

libPopulations.a:  StatPopFactory.o DynPopFactory.o $(POPS)
	ar $(ARFL) $@ $?

clean:
	rm -f *.o libPopulations.a *.ti StatPopFactory.cpp






# populations

%Pop.o: %Pop.cpp %Pop.h  
	 $(CC) $(COPT) $(DEFS) -I$(ACTIONS) -I$(ICOSA) -I$(IO) -I$(CORE) -I$(GENES) -I$(UTILS_QHG) -I $(GROUPS) -fPIC



