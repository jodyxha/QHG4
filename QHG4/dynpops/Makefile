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
ifdef OUT
    SO_DIR=$(OUT)
else
    SO_DIR=./so_dir
endif

WRPSRC=$(wildcard *Wrapper.cpp)
WRPS=$(WRPSRC:.cpp=.so)

clean:
lib: libs0
all: libs0

test: libs0 plugtest

libs0: 
	rm -f *PopWrapper.*
	./build_wrappers.py $(POPS)
	make libs

libs: $(WRPS)
	touch dummy.so
	mv *.so ${SO_DIR}
	rm -f ${SO_DIR}/dummy.so
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
        $(POPS)/$(subst  Wrapper.so,,$@).o -lhdf5

