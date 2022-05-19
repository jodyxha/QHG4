
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

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF)  $(CPROF) -I$(UTILS) -I$(ICOSA) $(COMP)


TARGS=EQTileTool edit_links


app: $(TARGS)
	$(if $(UTILS_DIR),cp $? $(UTILS_DIR),)


new: clean app

clean: 
	rm -f *.o $(TARGS)



#####
# EQTileTool
#####

EQNodeClassificator.o: EQNodeClassificator.cpp EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS)

g++ -g EQVertTri.cpp ../icosa/EQConnectivity.cpp  -I../icosa -I ../utils -L../icosa -lIcosa -L ../utils/ -lUtils -lzBasicTile.o: BasicTile.cpp BasicTile.h 
	$(CC) $(COPT) $(DEFS)

BasicSplitter.o: BasicSplitter.cpp BasicSplitter.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQTile.o: EQTile.cpp EQTile.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQSplitter.o: EQSplitter.cpp EQSplitter.h BasicSplitter.h EQTile.h EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS)

LonLatTile.o: LonLatTile.cpp LonLatTile.h BasicTile.h 
	$(CC) $(COPT) $(DEFS)

LonLatSplitter.o: LonLatSplitter.cpp LonLatSplitter.h BasicSplitter.h LonLatTile.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQZones.o: EQZones.cpp EQZones.h BasicTile.h
	$(CC) $(COPT) $(DEFS)

EQGridCreator.o: EQGridCreator.cpp EQGridCreator.h EQZones.h BasicTile.h BasicSplitter.h
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(CORE) -I$(ICOSA) -I$(IO)

EQTiling.o: EQTiling.cpp EQTiling.h BasicTiling.h EQSplitter.h EQGridCreator.h EQNodeClassificator.h
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(ICOSA)

LonLatTiling.o: LonLatTiling.cpp LonLatTiling.h BasicTiling.h LonLatSplitter.h EQGridCreator.h
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(ICOSA)

BasicTiling.o: BasicTiling.cpp BasicTiling.h BasicSplitter.h EQGridCreator.h
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(ICOSA)

EQTileLinks.o: EQTileLinks.cpp EQTileLinks.h 
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(ICOSA)

EQTileTool.o: EQTileTool.cpp EQGridCreator.h 
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(CORE) -I$(ICOSA) -I$(IO)

EQTileTool: EQTileTool.o EQGridCreator.o EQZones.o \
	BasicTile.o BasicSplitter.o BasicTiling.o \
	EQTile.o EQSplitter.o EQTiling.o \
        EQNodeClassificator.o EQTileLinks.o \
	LonLatTile.o LonLatSplitter.o LonLatTiling.o 
	$(CC) $(COMP) -o EQTileTool $(CADD) \
	EQTileTool.o  EQGridCreator.o EQZones.o \
	BasicTile.o BasicSplitter.o BasicTiling.o \
	EQTile.o EQSplitter.o EQTiling.o \
        EQNodeClassificator.o EQTileLinks.o \
	LonLatTile.o LonLatSplitter.o LonLatTiling.o \
	-L$(ICOSA) -lIcosa \
	-L$(IO) -lIO \
	-L$(CORE) -lCore \
	-L$(UTILS) -lUtils \
	-lhdf5 -lz

LinkTools.o: LinkTools.cpp LinkTools.h
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(CORE) -I$(ICOSA) -I$(IO)


edit_links.o: edit_links.cpp LinkTools.h
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(CORE) -I$(ICOSA) -I$(IO)

edit_links: edit_links.o LinkTools.o
	$(CC) $(COMP) -o edit_links  $(CADD) \
	edit_links.o LinkTools.o \
	-L$(ICOSA) -lIcosa \
	-L$(CORE) -lCore \
	-L$(ICOSA) -lIcosa \
	-L$(IO) -lIO \
	-L$(UTILS) -lUtils \
	-lhdf5 -lz -lcrypto

EQVertTri.o: EQVertTri.cpp ${ICOSA}/EQConnectivity.h 
	$(CC) $(COPT) $(DEFS) -I$(UTILS) -I$(CORE) -I$(ICOSA) -I$(IO)


EQVertTri: EQVertTri.o 
	${CC} $(COMP) -o EQVertTri \
	EQVertTri.o  \
	-L$(ICOSA) -lIcosa \
        -L$(UTILS) -lUtils \
	-lz
#g++ -g EQVertTri.cpp ../icosa/EQConnectivity.cpp  -I../icosa -I ../utils -L../icosa -lIcosa -L ../utils/ -lUtils -lz
