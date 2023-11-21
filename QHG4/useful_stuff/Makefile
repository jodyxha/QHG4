
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

UTILS_OUT=~/utils

TARG=SPTest


COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(COMP)


CORE_LIB:
	$(MAKE) --directory=$(CORE) lib

UTILS_LIB:
	$(MAKE) --directory=$(UTILS) lib

IO_LIB:
	$(MAKE) --directory=$(IO) lib

all: app

app: count_cell_agents count_region_agents LonLat2Node QDF2Rect RemoveRegions hyb_extract hyb_extract_ico3 ymt_extract ymt_xextract SubSpaceTest HybHist



clean:
	rm -f *.o *.ti  $(app)

new: clean app


RegionTester.o: RegionTester.cpp RegionTester.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

AgentCounter.o: AgentCounter.cpp AgentCounter.h RegionTester.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

count_region_agents.o:count_region_agents.cpp AgentCounter.h RegionTester.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

count_region_agents:count_region_agents.o AgentCounter.o RegionTester.o
	$(CC) $(COMP) -o count_region_agents \
        count_region_agents.o AgentCounter.o RegionTester.o \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz

count_cell_agents.o:count_cell_agents.cpp AgentCounter.h RegionTester.h
	$(CC) $(COPT) $(DEFS) -I$(IO)

count_cell_agents:count_cell_agents.o AgentCounter.o RegionTester.o
	$(CC) $(COMP) -o count_cell_agents \
        count_cell_agents.o AgentCounter.o RegionTester.o \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz

LonLat2Node.o: LonLat2Node.cpp
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

LonLat2Node:LonLat2Node.o
	$(CC) $(COMP) -o LonLat2Node \
        LonLat2Node.o \
	-L$(IO)       -lIO \
	-L$(ICOSA)    -lIcosa \
	-L$(CORE)   -lCore \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz

QDFDataExtractor.o: QDFDataExtractor.cpp QDFDataExtractor.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA)

HDataSetCollector.o: HDataSetCollector.cpp HDataSetCollector.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA)

PseudoPopArray.o: PseudoPopArray.cpp PseudoPopArray.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA)

PseudoPopCounts.o: PseudoPopCounts.cpp PseudoPopCounts.h PseudoPopArray.h
	$(CC) $(COPT) $(DEFS)  -I$(IO) -I$(ICOSA)

PseudoPopManager.o: PseudoPopManager.cpp PseudoPopManager.h PseudoPopArray.h PseudoPopCounts.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA)

QDF2Rect.o: QDF2Rect.cpp QDFDataExtractor.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA)

QDF2Rect: QDF2Rect.o QDFDataExtractor.o HDataSetCollector.o AgentCounter.o \
        PseudoPopArray.o PseudoPopCounts.o PseudoPopManager.o
	$(CC)  $(COMP) -o QDF2Rect \
        QDF2Rect.o QDFDataExtractor.o HDataSetCollector.o AgentCounter.o \
        PseudoPopArray.o PseudoPopCounts.o PseudoPopManager.o \
	-L$(IO)       -lIO \
	-L$(ICOSA)    -lIcosa \
	-L$(CORE)   -lCore \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz

RegionRemover.o: RegionRemover.cpp RegionRemover.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA)

RemoveRegions.o: RemoveRegions.cpp RegionRemover.h 
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ICOSA)

RemoveRegions: RegionRemover.o RemoveRegions.o
	$(CC)  $(COMP) -o RemoveRegions \
	RegionRemover.o RemoveRegions.o \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5

AgentHybCollector.o: AgentHybCollector.cpp AgentHybCollector.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA)

collect_agent_hybs.o: collect_agent_hybs.cpp AgentHybCollector.h
	$(CC) $(COPT) $(DEFS)	

collect_agent_hybs: collect_agent_hybs.o AgentHybCollector.o	
	$(CC)  $(COMP) -o collect_agent_hybs \
	collect_agent_hybs.o AgentHybCollector.o \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz $(COMP)

AgentItemCollector.o: AgentItemCollector.cpp AgentItemCollector.h
	$(CC) $(COPT)  $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) 

AgentYMTCollector.o: AgentYMTCollector.cpp AgentYMTCollector.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) 

AgentXItemCollector.o: AgentXItemCollector.cpp AgentXItemCollector.h
	$(CC) $(COPT)  $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) 

bin_agent_items.o: bin_agent_items.cpp AgentItemCollector.h
	$(CC) $(COPT)  $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) 

bin_agent_items: bin_agent_items.o AgentItemCollector.o	
	$(CC) $(COMP) -o bin_agent_items \
	bin_agent_items.o AgentItemCollector.o \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz 


HybHDFWriter.o: HybHDFWriter.cpp HybHDFWriter.h AgentItemCollector.h
	$(CC) $(COPT)  $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) -I${GENES}

YMTHDFWriter.o: YMTHDFWriter.cpp YMTHDFWriter.h AgentYMTCollector.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) -I${GENES}

XHDFWriter.o: XHDFWriter.cpp XHDFWriter.h AgentXItemCollector.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) -I${GENES}

hyb_extract_ico3.o: hyb_extract_ico3.cpp AgentItemCollector.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) -I${GENES}

hyb_extract_ico3: hyb_extract_ico3.o AgentItemCollector.o	HybHDFWriter.o
	$(CC) $(COMP) -o hyb_extract_ico3 \
	hyb_extract_ico3.o AgentItemCollector.o HybHDFWriter.o \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz 


CellSampler.o: CellSampler.cpp CellSampler.h AgentItemCollector.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) -I${GENES}

hyb_extract.o: hyb_extract.cpp AgentItemCollector.h CellSampler.h HybHDFWriter.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA)  -I${GENES}

hyb_extract: hyb_extract.o AgentItemCollector.o CellSampler.o HybHDFWriter.o
	$(CC) $(COMP) -o hyb_extract \
	hyb_extract.o AgentItemCollector.o CellSampler.o HybHDFWriter.o \
	-L${GENES} -lGenes \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz 


ymt_extract.o: ymt_extract.cpp AgentYMTCollector.h CellSampler.h YMTHDFWriter.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA)  -I${GENES}

ymt_extract: ymt_extract.o AgentYMTCollector.o CellSampler.o YMTHDFWriter.o
	$(CC) $(COMP) -o ymt_extract \
	ymt_extract.o AgentYMTCollector.o CellSampler.o YMTHDFWriter.o \
	-L${GENES} -lGenes \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz 



ymt_xextract.o: ymt_xextract.cpp AgentXItemCollector.h CellSampler.h XHDFWriter.h
	$(CC) $(COPT)  $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA)  -I${GENES}

ymt_xextract: ymt_xextract.o AgentXItemCollector.o CellSampler.o XHDFWriter.o
	$(CC) $(COMP) -o ymt_xextract \
	ymt_xextract.o AgentXItemCollector.o CellSampler.o XHDFWriter.o \
	-L${GENES} -lGenes \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz 

hdf_catenate.o: hdf_catenate.cpp
	$(CC) $(COPT)  $(DEFS) -I$(IO) 


hdf_catenate: hdf_catenate.o
	$(CC) $(COMP) -o hdf_catenate \
	hdf_catenate.o \
	-L$(IO)       -lIO \
	-L$(UTILS)   -lUtils \
	-lhdf5 



SubSpace.o: SubSpace.cpp SubSpace.h
	$(CC) $(COPT) $(DEFS) -I$(IO) 

SubSpaceTest.o: SubSpaceTest.cpp SubSpace.h
	$(CC) $(COPT) $(DEFS) -I$(IO) 

SubSpaceTest: SubSpaceTest.o SubSpace.o
	$(CC) $(COMP)  -o SubSpaceTest \
	SubSpaceTest.o SubSpace.o \
	-L$(UTILS)   -lUtils -lz


hdfutils.o: hdfutils.cpp hdfutils.h
	$(CC) $(COPT) $(DEFS)

HybCollector.o: HybCollector.cpp HybCollector.h hdfutils.h
	$(CC) $(COPT) $(DEFS)

HybHist.o: HybHist.cpp HybCollector.h SubSpace.h
	$(CC) $(COPT) $(DEFS)

HybHist: HybHist.o HybCollector.o hdfutils.o SubSpace.o 
	$(CC) $(COMP) -o HybHist \
	HybHist.o HybCollector.o hdfutils.o SubSpace.o \
	-L$(UTILS)   -lUtils \
	-lhdf5 

CSVXMLChecker.o: CSVXMLChecker.cpp CSVXMLChecker.h
	$(CC) $(COPT) $(DEFS) -I$(IO) 

check_csv_xml.o: check_csv_xml.cpp CSVXMLChecker.h
	$(CC) $(COPT) $(DEFS) -I$(IO) 

check_csv_xml: check_csv_xml.o CSVXMLChecker.o
	$(CC) $(COMP) -o check_csv_xml \
	check_csv_xml.o CSVXMLChecker.o \
	-L$(UTILS)   -lUtils \
        -lz
