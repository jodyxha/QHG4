
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

TARGETS=Def2QDF QDF2PNG QDF2PNGNew txt2xml


COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(COMP)

CORE_LIB:
	$(MAKE) --directory=$(CORE) lib

UTILS_LIB:
	$(MAKE) --directory=$(UTILS) lib


ICOSA_LIB:
	$(MAKE) --directory=$(ICOSA) lib

IO_LIB:
	$(MAKE) --directory=$(IO) lib

HEADER_LIB:
	$(MAKE) --directory=$(HEADER) 




all: app




app: $(TARGETS)
	$(if $(UTILS2),cp $? $(UTILS2),)


clean:
	rm -f *.o *.ti $(TARGETS)

new: clean app


Def2QDF.o: Def2QDF.cpp  
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(IO)

GridFactory.o: GridFactory.cpp GridFactory.h NodeIndex.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(IO) -I$(UTILS_QHG)


Def2QDF: Def2QDF.o GridFactory.o
	$(CC) $(COMP) -o Def2QDF \
        Def2QDF.o GridFactory.o \
	-L$(IO)       -lIO \
	-L$(CORE)     -lCore \
	-L$(ICOSA)    -lIcosa \
	-L$(UTILS)    -lUtils \
	-lhdf5 -lz 

CreateEmptyQDF.o: CreateEmptyQDF.cpp  
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(IO)


CreateEmptyQDF: CreateEmptyQDF.o 
	$(CC) $(COMP) -o CreateEmptyQDF \
        CreateEmptyQDF.o \
^	-L$(IO)       -lIO \
	-L$(CORE)     -lCore \
	-L$(ICOSA)    -lIcosa \
	-L$(UTILS)    -lUtils \
	-lhdf5 -lz


SurfaceGrid.o: SurfaceGrid.cpp SurfaceGrid.h 
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(IO)

QDF2PNG.o: QDF2PNG.cpp SurfaceGrid.h 
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(VISUAL) -I$(IO)  `pkg-config --cflags gdk-2.0 pangocairo` -I /usr/include/gtk-2.0/gdk
QDF2PNG: QDF2PNG.o SurfaceGrid.o
	$(CC) $(COMP) -o QDF2PNG \
	QDF2PNG.o SurfaceGrid.o \
	-L$(IO)       -lIO \
	-L$(CORE)     -lCore \
	-L$(ICOSA)    -lIcosa \
	-L$(UTILS)    -lUtils \
	-L$(VISUAL)   -lVisual \
        -lz -lhdf5 -lpng \
        `pkg-config --libs gdk-2.0 pangocairo`

QDFImageExtractor.o: QDFImageExtractor.cpp QDFImageExtractor.h SurfaceGrid.h  
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(VISUAL) -I$(IO)  `pkg-config --cflags gdk-2.0 pangocairo` -I /usr/include/gtk-2.0/gdk

QDF2PNGNew.o: QDF2PNGNew.cpp QDFImageExtractor.h 
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA)  -I$(IO)  `pkg-config --cflags gdk-2.0 pangocairo` -I /usr/include/gtk-2.0/gdk $(COMP)
QDF2PNGNew: QDF2PNGNew.o QDFImageExtractor.o SurfaceGrid.o
	$(CC) $(COMP) -o QDF2PNGNew \
	QDF2PNGNew.o QDFImageExtractor.o SurfaceGrid.o \
	-L$(IO)       -lIO \
	-L$(CORE)     -lCore \
	-L$(ICOSA)    -lIcosa \
	-L$(UTILS)    -lUtils \
	-L$(VISUAL)   -lVisual \
        -lz -lhdf5 -lpng \
        `pkg-config --libs gdk-2.0 pangocairo` $(COMP)


Angulator.o: Angulator.cpp Angulator.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(IO)  

Angulator: Angulator.o
	$(CC) $(COMP) -o Angulator \
	Angulator.o \
	-L$(IO)       -lIO \
	-L$(CORE)     -lCore \
	-L$(ICOSA)    -lIcosa \
	-L$(UTILS)    -lUtils \
        -lz -lhdf5 


makeNEMOlist.o: makeNEMOlist.cpp SurfaceGrid.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ICOSA) -I$(IO)  

makeNEMOlist: makeNEMOlist.o SurfaceGrid.o
	$(CC) $(COMP) -o makeNEMOlist \
        makeNEMOlist.o SurfaceGrid.o \
	-L$(IO)       -lIO \
	-L$(CORE)     -lCore \
	-L$(ICOSA)    -lIcosa \
	-L$(UTILS)    -lUtils \
        -lz -lhdf5 

txt2xml.o: txt2xml.cpp
	$(CC) $(COPT) $(DEFS) 


txt2xml: txt2xml.o
	$(CC) $(COMP) -o txt2xml \
	txt2xml.o \
	-L $(UTILS) -lUtils \
	-lz

ImageStamper.o: ImageStamper.cpp
	$(CC) $(COPT) $(DEFS)   -I$(VISUAL) -I$(ICOSA) \
        `pkg-config gtkmm-2.4 --cflags`	

ImageStamper: ImageStamper.o
	$(CC) $(COMP) -o ImageStamper \
	ImageStamper.o \
	-L$(VISUAL)   -lVisual \
	-L$(UTILS)    -lUtils  \
        -lpng $(COMP) \
        `pkg-config --libs gdk-2.0 pangocairo`


IDS_AND_POPFACTORY:
	$(POPULATIONS)/configure.sh


SimLoader.o: SimLoader.cpp SimLoader.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS) -I$(UTILS_QHG)

QDF4Convert.o: QDF4Convert.cpp SimLoader.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ACTIONS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)


QDF4Convert: QDF4Convert.o SimLoader.o \
    IDS_AND_POPFACTORY 
	$(CC) $(COMP) $(CFLAGS)  -o QDF4Convert \
	QDF4Convert.o SimLoader.o \
	-L $(POPULATIONS) -lPopulations \
	-L $(GENES)       -lGenes \
	-L $(CORE)        -lCore \
	-L $(ACTIONS)     -lActions \
	-L $(ICOSA)       -lIcosa \
	-L $(GEOINFO)     -lGeoInfo \
	-L $(UTILS)       -lUtils \
	-L $(UTILS_QHG)   -lUtilsQHG \
	-L $(IO)          -lIO \
        -lz -lhdf5 -lm -lcrypto 



PopFiller.o: PopFiller.cpp
	$(CC) $(COPT) $(DEFS)  -I$(CORE) -I$(ACTIONS) -I$(QMAPS) -I$(ICOSA) -I$(IO) -I$(POPULATIONS)

PopFiller: PopFiller.o
	$(CC) $(COMP) $(CFLAGS)  -o PopFiller \
	PopFiller.o \
	-L $(POPULATIONS) -lPopulations \
	-L $(GENES)       -lGenes \
	-L $(IO)          -lIO \
	-L $(CORE)        -lCore \
	-L $(ACTIONS)     -lActions \
	-L $(ICOSA)       -lIcosa \
	-L $(UTILS)       -lUtils \
        -lz -lhdf5 -lm -lcrypto 

CubemapImages.o: CubemapImages.cpp CubemapImages.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(IO) -I$(VISUAL)

QDF2CubemapImages.o: QDF2CubemapImages.cpp CubemapImages.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(IO) -I$(VISUAL)

QDF2CubemapImages: QDF2CubemapImages.o CubemapImages.o
	$(CC) $(COMP) $(CFLAGS)  -o QDF2CubemapImages \
         QDF2CubemapImages.o CubemapImages.o \
	-L$(ICOSA)  -lIcosa \
	-L$(IO)     -lIO \
	-L$(VISUAL) -lVisual \
	-L$(UTILS)  -lUtils \
        -lhdf5 -lz -lpng

LookUpSampler.o: LookUpSampler.cpp LookUpSampler.h
	$(CC) $(COPT) $(DEFS)  -I$(VISUAL)


LookUpSamples.o: LookUpSamples.cpp LookUpSampler.h
	$(CC) $(COPT) $(DEFS)  -I$(VISUAL)

LookUpSamples: LookUpSamples.o LookUpSampler.o
	$(CC) $(COMP) $(CFLAGS)  -o LookUpSamples \
        LookUpSamples.o LookUpSampler.o \
       -L$(VISUAL) -lVisual \
       -L$(UTILS)  -lUtils \
       -lpng  




