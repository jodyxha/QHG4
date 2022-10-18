
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



COPT=-c $< -o $@ $(CADD) $(CFLAGS) $(C_STD) $(CRAY_DEF) $(DYN) -fPIC -I$(UTILS) $(COMP)


$(warning DEFS in utils is $(DEFS))


ARFL=rv


lib: libUtilsQHG.a

all: libUtilsQHG.a

utils_qhg: libUtilsQHG.a


libUtilsQHG.a:  Triggers.o EventConsts.o EventData.o EventManager.o EventChecker.o
	ar $(ARFL) $@ $?


test: $(TESTS)


clean:
	rm -f *.o libUtilsQHG.a *.ti $(TESTS)


Triggers.o: Triggers.cpp Triggers.h 
	$(CC) $(COPT) $(DEFS)

EventConsts.o: EventConsts.cpp EventConsts.h
	$(CC) $(COPT) $(DEFS)

EventData.o: EventData.cpp EventData.h
	$(CC) $(COPT) $(DEFS)

EventManager.o: EventManager.cpp EventManager.h EventData.h Triggers.h
	$(CC) $(COPT) $(DEFS)

EventChecker.o: EventChecker.cpp EventChecker.h
	$(CC) $(COPT) $(DEFS)  -I$(CORE)  -I$(ICOSA) -I$(IO) -I$(POPULATIONS)


