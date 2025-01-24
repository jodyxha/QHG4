
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


UTILS2=~/utils

TARGETS=a2dtest a2dsamptest a3dtest SamplingTest


COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(COMP)


all: app




app: $(TARGETS)


clean:
	rm -f *.o *.ti $(TARGETS)

new: clean app



Sampling.o: Sampling.cpp Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

FullSampling.o: FullSampling.cpp FullSampling.h  Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

EachSampling.o: EachSampling.cpp EachSampling.h  Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

CoordRangeSampling.o: CoordRangeSampling.cpp CoordRangeSampling.h Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

CellRangeSampling.o: CellRangeSampling.cpp CellRangeSampling.h Sampling.h SamplingT.cpp
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

GridRangeSampling.o: GridRangeSampling.cpp GridRangeSampling.h Sampling.h SamplingT.cpp
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

SamplingTest.o: SamplingTest.cpp Sampling.h  FullSampling.h  CoordRangeSampling.h CellRangeSampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

SamplingTest: SamplingTest.o Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o
	$(CC) $(COMP) -o SamplingTest \
        SamplingTest.o Sampling.o FullSampling.o EachSampling.o CoordRange.o CellRangeSampling.o \
	-L$(IO)       -lIO \
	-L$(UTILS)    -lUtils

Agent2DataExtractor.o: Agent2DataExtractor.cpp Agent2DataExtractor.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

a2dtest.o: a2dtest.cpp Agent2DataExtractor.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

a2dtest: a2dtest.o  Agent2DataExtractor.o
	$(CC) $(COMP) -o a2dtest \
        a2dtest.o Agent2DataExtractor.o \
	-L$(IO)       -lIO \
	-L$(UTILS)    -lUtils \
        -lhdf5 -std=c++20

Agent3DataExtractor.o: Agent3DataExtractor.cpp Agent3DataExtractor.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

a3dtest.o: a3dtest.cpp Agent3DataExtractor.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

a3dtest: a3dtest.o  Agent3DataExtractor.o
	$(CC) $(COMP) -o a3dtest \
        a3dtest.o Agent3DataExtractor.o \
	-L$(IO)       -lIO \
	-L$(UTILS)    -lUtils \
        -lhdf5 -std=c++20

HistoMaker.o: HistoMaker.cpp HistoMaker.h Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

PieWriter.o: PieWriter.cpp PieWriter.h Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)


a2dsamptest.o: a2dsamptest.cpp Agent2DataExtractor.h  Sampling.h  FullSampling.h EachSampling.h  CoordRangeSampling.h CellRangeSampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

a2dsamptest: a2dsamptest.o  Agent2DataExtractor.o Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o HistoMaker.o PieWriter.o
	$(CC) $(COMP) -o a2dsamptest \
        a2dsamptest.o Agent2DataExtractor.o Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o HistoMaker.o PieWriter.o \
	-L$(IO)       -lIO \
	-L$(UTILS)    -lUtils \
        -lhdf5 -std=c++20

makeAgentDataHistoPie.o: makeAgentDataHistoPie.cpp Agent2DataExtractor.h  Sampling.h  FullSampling.h EachSampling.h  CoordRangeSampling.h CellRangeSampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

makeAgentDataHistoPie: makeAgentDataHistoPie.o  Agent2DataExtractor.o   Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o HistoMaker.o PieWriter.o
	$(CC) $(COMP) -o makeAgentDataHistoPie \
        makeAgentDataHistoPie.o Agent2DataExtractor.o  Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o HistoMaker.o PieWriter.o \
	-L$(IO)       -lIO \
	-L$(UTILS)    -lUtils \
        -lhdf5 -lz -std=c++20

AgentDataHistoPie.o: AgentDataHistoPie.cpp AgentDataHistoPie.h Agent2DataExtractor.h  Sampling.h  FullSampling.h  EachSampling.h CoordRangeSampling.h CellRangeSampling.h HistoMaker.h PieWriter.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

adhp.o: adhp.cpp AgentDataHistoPie.h Agent2DataExtractor.h  Sampling.h  FullSampling.h EachSampling.h  CoordRangeSampling.h CellRangeSampling.h HistoMaker.h PieWriter.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

adhp: adhp.o  AgentDataHistoPie.o Agent2DataExtractor.o  Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o GridRangeSampling.o HistoMaker.o PieWriter.o
	$(CC) $(COMP) -o adhp \
        adhp.o AgentDataHistoPie.o Agent2DataExtractor.o  Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o GridRangeSampling.o HistoMaker.o PieWriter.o \
        -L$(IO)       -lIO \
        -L$(UTILS)    -lUtils \
        -lhdf5 -lz -std=c++20

HistoMaker_multi.o: HistoMaker_multi.cpp HistoMaker_multi.h Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

PieWriter_multi.o: PieWriter_multi.cpp PieWriter_multi.h Sampling.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

AgentDataHistoPie_multi.o: AgentDataHistoPie_multi.cpp AgentDataHistoPie_multi.h Agent2DataExtractor.h  Sampling.h  FullSampling.h  EachSampling.h CoordRangeSampling.h CellRangeSampling.h HistoMaker_multi.h PieWriter_multi.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

adhp_multi.o: adhp_multi.cpp AgentDataHistoPie_multi.h Agent2DataExtractor.h  Sampling.h  FullSampling.h EachSampling.h  CoordRangeSampling.h CellRangeSampling.h HistoMaker_multi.h PieWriter_multi.h
	$(CC) $(COPT) $(DEFS)  -I$(UTILS) -I$(IO)

adhp_multi: adhp_multi.o  AgentDataHistoPie_multi.o Agent2DataExtractor.o  Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o GridRangeSampling.o HistoMaker_multi.o PieWriter_multi.o
	$(CC) $(COMP) -o adhp_multi \
        adhp_multi.o AgentDataHistoPie_multi.o Agent2DataExtractor.o  Sampling.o FullSampling.o EachSampling.o CoordRangeSampling.o CellRangeSampling.o GridRangeSampling.o HistoMaker_multi.o PieWriter_multi.o \
	-L$(IO)       -lIO \
	-L$(UTILS)    -lUtils \
        -lhdf5 -lz -std=c++20


