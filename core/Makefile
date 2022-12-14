
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


#ifdef OLDBIRTHS
#  CFLAGS:=$(CFLAGS) -DOLDBIRTHS
#endif 

ifdef OLDBIRTHS
  DOLD=-DOLDBIRTHS
endif 


COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(DOLD) $(DYN) $(COMP) -fPIC

ARFL=rv


all: core

lib: libCore.a

core:  libCore.a

libCore.a:  Geography.o Climate.o Vegetation.o \
	      NPPCalc.o NPPCalcMiami.o NPPCalcNCEAS.o \
              MoveStats.o Navigation.o GridFactory.o SCellGrid.o OccTracker.o OccHistory.o PopLooper.o  \
	      SPopulation.o Prioritizer.o EnvInterpolator.o AutoInterpolator.o \
              clsutils.o
	ar $(ARFL) $@ $?


clean:
	rm -f *.o libCore.a *.ti


Geography.o: Geography.cpp Geography.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)

Climate.o: Climate.cpp Climate.h Geography.h
	$(CC) $(COPT) $(DEFS) -I$(QMAPS)  -I$(ICOSA) -I$(UTILS_QHG)

Vegetation.o: Vegetation.cpp Vegetation.h Climate.h Geography.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA) 

NPPCalc.o: NPPCalc.cpp NPPCalc.h Climate.h Geography.h
	$(CC) $(COPT) $(DEFS)  

NPPCalcMiami.o: NPPCalcMiami.cpp NPPCalcMiami.h NPPCalc.h Climate.h Geography.h
	$(CC) $(COPT) $(DEFS)  

NPPCalcNCEAS.o: NPPCalcNCEAS.cpp NPPCalcNCEAS.h NPPCalc.h Climate.h Geography.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA) 

MoveStats.o: MoveStats.cpp MoveStats.h 
	$(CC) $(COPT) $(DEFS) -I$(IO) 

Navigation.o: Navigation.cpp Navigation.h 
	$(CC) $(COPT) $(DEFS) 

GridFactory.o: GridFactory.cpp GridFactory.h 
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(GEOINFO) -I$(UTILS_QHG)

SCellGrid.o: SCellGrid.cpp SCellGrid.h SCell.h Geography.h Climate.h
	$(CC) $(COPT) $(DEFS) -I$(ICOSA)

SPopulation.o: SPopulation.cpp SPopulation.h 
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(IO)

Prioritizer.o: Prioritizer.cpp Prioritizer.h $(POPS)
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(IO) -I$(ACTIONS)


OccHistory.o: OccHistory.cpp OccHistory.h 
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(IO)

OccTracker.o: OccTracker.cpp OccTracker.h OccHistory.h 
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(IO)


PopLooper.o: PopLooper.cpp PopLooper.h PopBase.h clsutils.h 
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA) -I$(IO)

clsutils.o: clsutils.cpp clsutils.h
	$(CC) $(COPT) $(DEFS)

EnvInterpolator.o: EnvInterpolator.cpp EnvInterpolator.h SCellGrid.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)  -I$(IO)  -I$(UTILS_QHG)


AutoInterpolator.o: AutoInterpolator.cpp AutoInterpolator.h SCellGrid.h
	$(CC) $(COPT) $(DEFS)  -I$(ICOSA)  -I$(IO)  -I$(UTILS_QHG)


