
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifdef OPT
  CFLAGS=-O3 $(WARNINGS) 
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


UTIL_DIR=~/utils

COPT=-c -g $< -I$(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(COMP)

APPS=Coastal VectorImport SeaWays NavToNav2

app: $(APPS)

all: $(APPS)

libImport.a: TabReader.o TabDataReader.o UDelReader.o FAOReader.o
	ar $(ARFL) $@ $?

new: clean app

clean:
	rm -f *.o *.a *.ti libImport.a $(APPS)

dbfReader.o: dbfReader.cpp dbfReader.h 
	$(CC) $(COPT) $(DEFS)  

shpUtils.o: shpUtils.cpp shpUtils.h 
	$(CC) $(COPT) $(DEFS)

shpRecord.o: shpRecord.cpp shpRecord.h shpUtils.h 
	$(CC) $(COPT) $(DEFS) 

shpHeader.o: shpHeader.cpp shpHeader.h shpUtils.h 
	$(CC) $(COPT) $(DEFS) 

VectorImport.o: VectorImport.cpp shpHeader.h shpUtils.h shpRecord.h dbfReader.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(IO) -I$(GEOINFO)

VectorImport:VectorImport.o shpHeader.o shpUtils.o shpRecord.o dbfReader.o
	$(CC) $(COMP) -g -Wall -o VectorImport VectorImport.o shpHeader.o shpUtils.o shpRecord.o dbfReader.o \
	-L $(CORE)    -lCore \
	-L $(ICOSA)   -lIcosa \
	-L $(GEOINFO) -lGeoInfo \
	-L $(IO)      -lIO \
	-L $(UTILS)   -lUtils \
	-lz -lhdf5


Coastal.o: Coastal.cpp 
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(GEOINFO)

Coastal: Coastal.o
	$(CC) $(COMP) -g -Wall -o Coastal Coastal.o \
	-L $(CORE)    -lCore \
	-L $(ICOSA)   -lIcosa \
	-L $(IO)      -lIO \
	-L $(UTILS)   -lUtils \
	-lz -lhdf5



TabReader.o:TabReader.cpp TabReader.h
	$(CC) $(COPT) $(DEFS)

TabDataReader.o:TabDataReader.cpp TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS)

UDelReader.o:UDelReader.cpp UDelReader.h TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS) -I $(QMAPS)

FAOReader.o:FAOReader.cpp FAOReader.h TabDataReader.h TabDataReader.h TabReader.h
	$(CC) $(COPT) $(DEFS)

UDel2QMap.o:UDel2QMap.cpp
	$(CC) $(COPT) $(DEFS) -I $(QMAPS)

UDel2QMap:UDel2QMap.o \
		UDelReader.o TabDataReader.o TabReader.o \
		$(QMAPS)/QMapHeader.o $(QMAPS)/QMapReader.o $(QMAPS)/QMapUtils.o
	$(CC) $(COMP) -o UDel2QMap \
        UDel2QMap.o \
        UDelReader.o \
        TabDataReader.o \
        TabReader.o \
	$(QMAPS)/QMapHeader.o \
        $(QMAPS)/QMapReader.o \
        $(QMAPS)/QMapUtils.o \
        $(UTILS)/strutils.o $(UTILS)/BinSearch.o $(UTILS)/Interpolator_gsl.o


ComponentBuilder.o: ComponentBuilder.cpp ComponentBuilder.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA) 

CoastalDistances.o: CoastalDistances.cpp CoastalDistances.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA) 

SubComponent.o: SubComponent.cpp SubComponent.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA) 

SubComponentManager.o: SubComponentManager.cpp SubComponentManager.h SubComponent.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA) 

SeaWays.o: SeaWays.cpp ComponentBuilder.h CoastalDistances.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(IO) -I$(ICOSA) 


SeaWays: SeaWays.o ComponentBuilder.o CoastalDistances.o SubComponentManager.o SubComponent.o
	$(CC) $(COMP) -o SeaWays \
	SeaWays.o ComponentBuilder.o CoastalDistances.o SubComponentManager.o SubComponent.o \
	-L$(ICOSA)  -lIcosa \
	-L$(IO)     -lIO \
	-L$(CORE)   -lCore \
	-L$(UTILS)  -lUtils \
	-lhdf5 -lz

NavToNav2Converter.o: NavToNav2Converter.cpp NavToNav2Converter.h
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA) -I$(IO)

NavToNav2.o: NavToNav2.cpp 
	$(CC) $(COPT) $(DEFS) -I$(CORE) -I$(ICOSA) -I$(IO)

NavToNav2: NavToNav2.o NavToNav2Converter.o
	$(CC) $(COMP) -o NavToNav2 \
	NavToNav2.o NavToNav2Converter.o \
        -L$(ICOSA)  -lIcosa \
        -L$(IO)     -lIO \
        -L$(CORE)   -lCore \
        -L$(UTILS)  -lUtils \
        -lhdf5



