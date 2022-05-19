ifndef WARNINGS
  WARNINGS=-Wall 
endif

CFLAGS:=$(CFLAGS) $(WARNINGS)

ifdef OPT
  CFLAGS:=$(CFLAGS) -O3
else
  CFLAGS:=$(CFLAGS) -g

  ifdef PROF
     CFLAGS:=$(CFLAGS) -pg
  endif
endif 


ifdef STRICT
  CFLAGS:=$(CFLAGS) -Wconversion
endif	

ifdef DYN
  CFLAGS:=$(CFLAGS) -rdynamic
  DYN=-DDYN
endif	