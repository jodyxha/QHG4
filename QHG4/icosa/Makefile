
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

ifndef WARNINGS
  WARNINGS=-Wall 
endif

include ../craydefs.mk
include ../cflags.mk
include ../codedirs.mk


QMAPS=../qmaps


ifdef OPT
  CFLAGS=-O3 $(WARNINGS)
endif
ifdef PROF
  CPROF=-pg
else
  CPROF=
endif

ifndef ARFL
  ARFL=-rv
endif

COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF)  $(CPROF) -I$(UTILS) $(COMP)

CORE=IcoGridNodes.o IcoNode.o \
     EQTriangle.o Tegmark.o EQsahedron.o  \
     IcoFace.o SimpleFace.o QuadFace.o TriFace.o \
     Icosahedron.o IcoHeader.o VertexLinkage.o symbuf.o Lattice.o \
     EQConnectivity.o


all: libIcosa.a 



lib: libIcosa.a


new: clean all

clean: 
	rm -f *.o libIcosa.a

libIcosa.a: $(CORE)
	ar $(ARFL) $@ $?



Lattice.o: Lattice.cpp Lattice.h  
	$(CC) $(COPT) $(DEFS) -I$(GEOINFO)

SimpleFace.o: SimpleFace.cpp SimpleFace.h
	$(CC) $(COPT) $(DEFS) 

IcoFace.o: IcoFace.cpp IcoFace.h 
	$(CC) $(COPT) $(DEFS) -I$(QMAPS)

QuadFace.o: QuadFace.cpp QuadFace.h SimpleFace.h PolyFace.h 
	$(CC) $(COPT) $(DEFS)

TriFace.o: TriFace.cpp TriFace.h SimpleFace.h PolyFace.h 
	$(CC) $(COPT) $(DEFS)

VertexLinkage.o: VertexLinkage.cpp VertexLinkage.h IcoFace.h QuadFace.h icoutil.h
	$(CC) $(COPT) $(DEFS)

Icosahedron.o: Icosahedron.cpp Icosahedron.h IcoHeader.h PolyFace.h IcoFace.h icoutil.h \
	VertexLinkage.h 
	$(CC) $(COPT) $(DEFS)  

EQConnectivity.o: EQConnectivity.cpp EQConnectivity.h
	$(CC) $(COPT) $(DEFS)


IcoHeader.o: IcoHeader.cpp IcoHeader.h icoutil.h
	$(CC) $(COPT) $(DEFS)  

Tegmark.o:Tegmark.cpp Tegmark.h
	$(CC) $(COPT) $(DEFS) 

EQTriangle.o:EQTriangle.cpp EQTriangle.h Tegmark.h
	$(CC) $(COPT) $(DEFS) 

EQsahedron.o:EQsahedron.cpp EQsahedron.h EQTriangle.h Tegmark.h 
	$(CC) $(COPT) $(DEFS)  


IcoNode.o: IcoNode.cpp IcoNode.h icoutil.h
	$(CC) $(COPT) $(DEFS) 


symbuf.o: symbuf.cpp symbuf.h
	$(CC) $(COPT) $(DEFS) 


IcoGridNodes.o: IcoGridNodes.cpp IcoGridNodes.h IcoNode.h
	$(CC) $(COPT) $(DEFS) 


faceTest.o: faceTest.cpp IcoFace.h Face.h
	$(CC) $(COPT) $(DEFS) 


faceTest:faceTest.o Face.o IcoFace.o QuadFace.o Icosahedron.o VertexLinkage.o symbuf.o IcoHeader.o
	$(CC) -o faceTest $(CADD) \
	faceTest.o Face.o IcoFace.o QuadFace.o Icosahedron.o VertexLinkage.o  symbuf.o IcoHeader.o \
	$(UTILS)/Vec3D.o  $(UTILS)/Quat.o  $(UTILS)/geomutils.o $(UTILS)/strutils.o $(UTILS)/BufWriter.o $(UTILS)/BufReader.o















