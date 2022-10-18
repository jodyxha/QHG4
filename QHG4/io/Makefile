
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


UTILS=../utils
CORE=../core
ICOSA=../icosa

COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) -fPIC


$(warning DEFS in IO is $(DEFS))

ARFL=rv

all: lib
app: lib
lib: libIO.a



#          PopWriter.o PopReader.o GridWriter.o GridReader.o \
#          GeoWriter.o GeoReader.o ClimateWriter.o ClimateReader.o \
#          MoveStatWriter.o MoveStatGroupReader.o \
#          NavReader.o NavWriter.o StatusWriter.o

libIO.a:  QDFUtils.o QDFArray.o \
	PopReader.o PopWriter.o GroupReader.o \
	GridWriter.o GridGroupReader.o \
	GeoWriter.o GeoGroupReader.o \
	ClimateWriter.o ClimateGroupReader.o \
	VegGroupReader.o VegWriter.o \
	NavGroupReader.o NavWriter.o \
	OccGroupReader.o OccWriter.o StatusWriter.o \
	WELLDumpRestore.o SequenceIOUtils.o \
	PermDumpRestore.o qhgXML.o ParamProvider2.o
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libIO.a *.ti 


QDFUtils.o: QDFUtils.cpp QDFUtils.h
	$(CC) $(COPT) $(DEFS)

QDFArray.o: QDFArray.cpp QDFArray.h QDFUtils.h
	$(CC) $(COPT) $(DEFS)



PopWriter.o: PopWriter.cpp PopWriter.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

PopReader.o: PopReader.cpp PopReader.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

GroupReader.o: GroupReader.cpp GroupReader.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

GridWriter.o: GridWriter.cpp GridWriter.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

GridGroupReader.o: GridGroupReader.cpp GridGroupReader.h GroupReader.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

GeoWriter.o: GeoWriter.cpp GeoWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

GeoGroupReader.o: GeoGroupReader.cpp GeoGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

ClimateWriter.o: ClimateWriter.cpp ClimateWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

ClimateGroupReader.o: ClimateGroupReader.cpp ClimateGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

VegWriter.o: VegWriter.cpp VegWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

VegGroupReader.o: VegGroupReader.cpp VegGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

NavWriter.o: NavWriter.cpp NavWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

NavGroupReader.o: NavGroupReader.cpp NavGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

OccWriter.o: OccWriter.cpp OccWriter.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

OccGroupReader.o: OccGroupReader.cpp OccGroupReader.h GroupReader.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

StatusWriter.o: StatusWriter.cpp StatusWriter.h PopWriter.h GridWriter.h GeoWriter.h NavWriter.h OccWriter.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA)

WELLDumpRestore.o: WELLDumpRestore.cpp WELLDumpRestore.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

SequenceIOUtils.o: SequenceIOUtils.cpp SequenceIOUtils.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 


PermDumpRestore.o: PermDumpRestore.cpp PermDumpRestore.h 
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

RectSurf.o: RectSurf.cpp RectSurf.h 
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA)



GRTest.o: GRTest.cpp GroupReader.h GridGroupReader.h ClimateGroupReader.h VegGroupReader.h NavGroupReader.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) 

GRTest: GRTest.o GroupReader.o GridGroupReader.o GeoGroupReader.o ClimateGroupReader.o VegGroupReader.o NavGroupReader.o QDFUtils.o
	$(CC) -o GRTest \
	GRTest.o GroupReader.o GridGroupReader.o GeoGroupReader.o ClimateGroupReader.o VegGroupReader.o NavGroupReader.o QDFUtils.o \
	-L$(CORE)   -lCore \
	-L$(ICOSA)    -lIcosa \
	-L$(UTILS)   -lUtils \
	-lhdf5 -lz


qhgXML.o: qhgXML.cpp qhgXML.h
	$(CC) $(COPT) $(DEFS) -fPIC

ParamProvider2.o: ParamProvider2.cpp ParamProvider2.h qhgXML.h
	$(CC) $(COPT) $(DEFS)

xmlread3.o: xmlread3.cpp ParamProvider2.h qhgXML.h
	$(CC) $(COPT) $(DEFS) 

xmlread3: xmlread3.o ParamProvider2.o qhgXML.o
	$(CC) -o xmlread3 \
	xmlread3.o ParamProvider2.o qhgXML.o \
	-L $(UTILS) -lUtils \
	-lz 

testqhgxml.o: testqhgxml.cpp qhgXML.h
	$(CC) $(COPT) $(DEFS) -I$(TINYXML)


testqhgxml: testqhgxml.o qhgXML.o
	$(CC) -o testqhgxml \
	testqhgxml.o qhgXML.o \
	-L $(UTILS) -lUtils \
	-lz

