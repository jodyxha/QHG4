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


POPS=../populations
ifeq ("X$(SHORT)", "X1")
    POPS    := ../short_pop
else
    ifneq ("X$(SHORT)", "X")
        POPS    := ../$(SHORT)_pop
    endif
endif

$(warning pops is '$(POPS)')

ASH=../$(SHORT)_mod

INCL=-I$(UTILS) -I$(CORE) -I$(ICOSA) -I$(POPS) -I$(GENES) -I$(IO) -I$(ACTIONS) -I$(UTILS_QHG) -I$(GROUPS) -I$(ASH)

CADD=-g -Wall
COPT=-c $< -I $(UTILS) -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(DEFS)
CLNK=-ldl
COMP=$(GNU_OMP)



#DEFS=-I 
SO_DIR=./so_dir


WRPSRC=$(wildcard *Wrapper.cpp)
WRPS=$(WRPSRC:.cpp=.so)


all: libs0

test: libs0 plugtest

libs0: 
	make clean
	./build_wrappers.py $(POPS)
	make libs

libs: $(WRPS)
	mv *.so ${SO_DIR}

clean:
	rm -f *.o plugtest ${SO_DIR}/*.so *PopWrapper.*



plugtest.o: plugtest.cpp 
	$(CC) $(COPT) $(COMP) -I$(CORE) -I$(ICOSA) -I$(IO) -I$(POPS)

plugtest: plugtest.o 
	$(CC) -o plugtest $(CADD) $(CLNK) $(COMP)  \
	plugtest.o \
	-L$(IO)    -lIO \
	-L$(POPS)  -lPopulations \
	-L$(ICOSA) -lIcosa \
	-L$(CORE)  -lCore \
	-L$(UTILS) -lUtils -lz




%Wrapper.o: %Wrapper.cpp %Wrapper.h  
	 $(CC) $(COPT) $(INCL) -fPIC

%Wrapper.so: %Wrapper.o
	$(CC) -shared  -o $@ $< \
        $(POPS)/$(subst  Wrapper.so,,$@).o -lhdf5 \
	-L$(IO)     -lIO \
	-L$(CORE)   -lCore \
	-L$(UTILS)  -lUtils 

