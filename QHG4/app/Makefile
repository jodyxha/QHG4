
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

OUTFILE=QHGMain
ifdef OUT
    OUTFILE=$(OUT)
endif
 
FORCECLEAN=

ifeq ("X$(SHORT)", "X1")
    POPULATIONS := ${QHG4_DIR}/short_pop
    MODULAR     := ${QHG4_DIR}/short_mod
else
    ifneq ("X$(SHORT)", "X")
        POPULATIONS := ${QHG4_DIR}/$(SHORT)_pop
        ACTIONS     := ${QHG4_DIR}/$(SHORT)_mod
    endif
endif

ifdef DYNAMIC_POPS
    DEFS=-D DYNAMIC_POPS
endif

DYN_OPT=-ldl


TARGETS=QHGMain

COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(COMP) $(DYN)

CORE_LIB:
	$(MAKE) --directory=$(CORE) $(FORCECLEAN) $(LIBTARG)

ifdef DYNAMIC_POPS
PLUG_LIB:
	$(MAKE) --directory=$(PLUGIN_STUB) lib
else
ACTIONS_LIB:
	$(MAKE) --directory=$(ACTIONS) $(FORCECLEAN) $(LIBTARG)

POPULATION_LIB:
	$(MAKE) --directory=$(POPULATIONS) $(FORCECLEAN) $(LIBTARG)
endif

UTILS_LIB:
	$(MAKE) --directory=$(UTILS) lib 

UTILS_QHG_LIB:
	$(MAKE) --directory=$(UTILS_QHG) lib 

IO_LIB:
	$(MAKE) --directory=$(IO) lib

ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

IDS_AND_POPFACTORY:
	$(POPULATIONS)/configure.sh




all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS) AGENT_CLASS_SUMMARY.txt

cleanTemp:
	rm -f *.o AGENT_CLASS_SUMMARY.txt


GridScrambler.o: GridScrambler.cpp GridScrambler.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

SimParams.o: SimParams.cpp SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS) -I$(UTILS_QHG)

SimParamsPlug.o: SimParamsPlug.cpp SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(PLUG_STUB) -I$(UTILS_QHG)

Simulator.o: Simulator.cpp Simulator.h SimParams.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS) -I$(UTILS_QHG)

QHGMain.o: QHGMain.cpp Simulator.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ACTIONS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

#QHGMain: QHGMain.o Simulator.o SimParams.o GridScrambler.o \
#    IDS_AND_POPFACTORY 
ifndef DYNAMIC_POPS
QHGMain: QHGMain.o Simulator.o SimParams.o GridScrambler.o 
	$(CC) $(CFLAGS) $(COMP) $(CADD)  $(DYN_OPT)  -o $(OUTFILE) \
	QHGMain.o Simulator.o SimParams.o GridScrambler.o \
	-L $(POPULATIONS) -lPopulations \
	-L $(GENES) -lGenes \
	-L $(CORE) -lCore \
	-L $(ACTIONS) -lActions \
	-L $(ICOSA) -lIcosa \
	-L $(GEOINFO) -lGeoInfo \
	-L $(IO) -lIO \
	-L $(UTILS_QHG) -lUtilsQHG \
	-L $(UTILS) -lUtils \
        -lz -lhdf5 -lm -lcrypto
else
QHGMain: QHGMain.o Simulator.o SimParams.o GridScrambler.o \
    IDS_AND_POPFACTORY 
	$(CC) $(CFLAGS) $(COMP) $(DYN_OPT)  -o $(OUTFILE)  \
	QHGMain.o Simulator.o SimParams.o GridScrambler.o \
	-L $(PLUGIN_STUB) -lPluginStub \
	-L $(GENES) -lGenes \
	-L $(CORE) -lCore \
	-L $(ICOSA) -lIcosa \
	-L $(GEOINFO) -lGeoInfo \
	-L $(IO) -lIO \
	-L $(UTILS_QHG) -lUtilsQHG \
	-L $(UTILS) -lUtils \
        -lz -lhdf5 -lm -lcrypto  -Wl,--export-dynamic 
endif

QHGMainDynExp: QHGMain.o Simulator.o SimParams.o GridScrambler.o \
    IDS_AND_POPFACTORY 
	$(CC) $(CFLAGS) $(COMP) $(DYN_OPT)  -o $(OUTFILE)  \
	QHGMain.o Simulator.o SimParams.o GridScrambler.o \
	-L $(PLUGIN_STUB) -lPluginStub \
	-L $(GENES) -lGenes \
	-L $(CORE) -Wl,--whole-archive -lCore -Wl,--no-whole-archive  \
	-L $(ICOSA) -lIcosa \
	-L $(GEOINFO) -lGeoInfo \
	-L $(IO)  -Wl,--whole-archive -lIO -Wl,--no-whole-archive  \
	-L $(UTILS_QHG) -lUtilsQHG \
	-L $(UTILS) -Wl,--whole-archive  -lUtils  -Wl,--no-whole-archive \
        -lz -lhdf5 -lm -lcrypto -Wl,--export-dynamic



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



