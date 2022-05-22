
ifdef GCC
  CC=$(GCC)
else
  CC=~/utils/cgcc
endif

include ../craydefs.mk
include ../cflags.mk
include ../codedirs.mk

ifdef MUTE
  MUTEFLAG=-D MUTE
else
  MUTEFLAG=
endif


ifndef WARNINGS
  WARNINGS=-Wall 
endif



OLDCOPT=-c $< -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(DYN) -fPIC -I$(UTILS) $(COMP)
COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(COMP) -I $(UTILS) 


$(warning DEFS in utils is $(DEFS))


ARFL=rv


lib: libGroups.a

all: libGroups.a

libGroups.a: ChildManager.o GroupManager.o RandomGroupSplitter.o
	ar $(ARFL) $@ $?


TESTS=rgstest cmtest
test: $(TESTS)


clean:
	rm -f *.o libUtilsQHG.a *.ti $(TESTS)


GroupManager.o: GroupManager.cpp GroupManager.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ACTIONS)  -I $(POPULATIONS)  -I$(CORE) -I$(ICOSA) -I .

ChildManager.o: ChildManager.cpp ChildManager.h
	$(CC) $(COPT) $(DEFS)   -I$(IO) -I$(ACTIONS)  -I $(POPULATIONS)  -I$(CORE) -I$(ICOSA)  -I$(TOOLS_IO) -I .

RandomGroupSplitter.o: RandomGroupSplitter.cpp RandomGroupSplitter.h GroupSplitter.h
	$(CC) $(COPT) $(DEFS)   


rgstest.o: rgstest.cpp RandomGroupSplitter.h GroupSplitter.h
	$(CC) $(COPT) $(DEFS)   

rgstest: rgstest.o RandomGroupSplitter.o
	$(CC) -o rgstest  rgstest.o RandomGroupSplitter.o \
        -L../utils -lUtils \
        -lcrypto


cmtest.o: cmtest.cpp ChildManager.cpp ChildManager.h  RandomGroupSplitter.cpp RandomGroupSplitter.h GroupSplitter.h
	$(CC) $(COPT) $(DEFS) -I$(IO) -I$(ACTIONS)  -I $(POPULATIONS)  -I$(CORE) -I$(ICOSA) -I$(TOOLS_IO) -I .

cmtest: cmtest.o ChildManager.o  RandomGroupSplitter.o
	$(CC) -o cmtest cmtest.o ChildManager.o  ../tools_io/SurfaceGrid.o $(COMP) \
        -L$(ACTIONS) -lActions \
        -L$(POPULATIONS) -lPopulations \
        -L$(UTILS_QHG) -lUtilsQHG  \
        -L$(ICOSA) -lIcosa  \
        -L$(IO) -lIO  \
        -L$(CORE) -lCore  \
        -L$(UTILS) -lUtils \
	 -lhdf5 -lcrypto -lz
