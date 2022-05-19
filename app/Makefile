
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


FORCECLEAN=

ifeq ("X$(SHORT)", "X1")
    POPULATIONS := ../short_pop
    MODULAR     := ../short_mod
else
    ifneq ("X$(SHORT)", "X")
        POPULATIONS := ../$(SHORT)_pop
        ACTIONS     := ../$(SHORT)_mod
    endif
endif


DYN_OPT=-ldl


TARGETS=QHGMain

COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(COMP) $(DYN)

CORE_LIB:
	$(MAKE) --directory=$(CORE) $(FORCECLEAN) $(LIBTARG)

ACTIONS_LIB:
	$(MAKE) --directory=$(ACTIONS) $(FORCECLEAN) $(LIBTARG)

POPULATION_LIB:
	$(MAKE) --directory=$(POPULATIONS) $(FORCECLEAN) $(LIBTARG)

UTILS_LIB:
	$(MAKE) --directory=$(UTILS) lib 

IO_LIB:
	$(MAKE) --directory=$(IO) lib

ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

IDS_AND_POPFACTORY:
	./configure.sh




all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS) AGENT_CLASS_SUMMARY.txt


GridScrambler.o: GridScrambler.cpp GridScrambler.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

EventChecker.o: EventChecker.cpp EventChecker.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS) -I$(UTILS_QHG)

SimParams.o: SimParams.cpp SimParams.h EventChecker.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS) -I$(UTILS_QHG)

Simulator.o: Simulator.cpp Simulator.h SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS) -I$(UTILS_QHG)

QHGMain.o: QHGMain.cpp Simulator.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ACTIONS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

QHGMain: QHGMain.o Simulator.o SimParams.o EventChecker.o GridScrambler.o \
    IDS_AND_POPFACTORY 
	$(CC) $(CFLAGS) $(COMP) $(DYN_OPT)  -o QHGMain \
	QHGMain.o Simulator.o SimParams.o EventChecker.o GridScrambler.o \
	-L $(POPULATIONS) -lPopulations \
	-L $(GENES) -lGenes \
	-L $(CORE) -lCore \
	-L $(GROUPS) -lGroups \
	-L $(ACTIONS) -lActions \
	-L $(ICOSA) -lIcosa \
	-L $(GEOINFO) -lGeoInfo \
	-L $(UTILS) -lUtils \
	-L $(UTILS_QHG) -lUtilsQHG \
	-L $(IO) -lIO \
        -lz -lhdf5 -lm -lcrypto 

SimSimpleParams.o: SimSimpleParams.cpp SimSimpleParams.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

QDF4Convert.o: QDF4Convert.cpp SimSimpleParams.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ACTIONS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)


QDF4Convert: QDF4Convert.o SimSimpleParams.o \
    IDS_AND_POPFACTORY 
	$(CC) $(CFLAGS)  -o QDF4Convert \
	QDF4Convert.o SimSimpleParams.o \
	-L $(POPULATIONS) -lPopulations \
	-L $(GENES) -lGenes \
	-L $(IO) -lIO \
	-L $(CORE) -lCore \
	-L $(ACTIONS) -lActions \
	-L $(ICOSA) -lIcosa \
	-L $(GEOINFO) -lGeoInfo \
	-L $(UTILS) -lUtils \
	-L $(UTILS_QHG) -lUtilsQHG \
        -lz -lhdf5 -lm -lcrypto 



