#why doesn't this work automatically?
#export CSCS=_CSCS_

# essential library stuff
CORE      := core
UTILS     := utils
UTILS_QHG := utils_qhg
GEOINFO   := geoinfo
VISUAL    := visual
TILING    := tiling 
IMPORT    := import
ICOSA     := icosa
IO        := io
ACTIONS   := actions
POPS      := populations
DYNPOPS   := dynpops
GROUPS    := groups

GENES     := genes
TOOLS_IO  := tools_io
TOOLS_ICO := tools_ico
TOOLS_QDF := tools_qdf
BIOMES    := biomes
USEFUL    := useful_stuff

export CRAY_OMP=-h omp
export GNU_OMP=-fopenmp
export CRAY_STD=-h std=c++11
export GNU_STD=-std=c++20

ifdef CRAY
  export CRAY_DEF=-DCRAY
  export C_STD=$(CRAY_STD)
else
  export CRAY_DEF=
  export C_STD=$(GNU_STD)
endif

$(warning std cmd name is '$(C_STD)')


ifeq ("X$(SHORT)", "X1")
    POPS    := short_pop
    MODULAR := short_mod
else 
    ifneq ("X$(SHORT)", "X")
        POPS    := $(SHORT)_pop
	ACTIONS := $(SHORT)_mod
    endif
endif


HOSTNAME:=$(shell hostname)
$(warning host name is '$(HOSTNAME)')

ifeq "$(HOSTNAME)" "aim-triops"
  $(warning no defs)
  export DEFS=
else
  ifeq "$(HOSTNAME)" "aim-frog"
    $(warning no defs)	
    export DEFS=
  else
    $(warning hdf5 defs)
    export DEFS=
  endif
endif

ifdef CRAY
  LIBRARIES := $(UTILS) $(UTILS_QHG) $(CORE) $(ACTIONS) $(GEOINFO) $(ICOSA) $(IO) $(GENES) $(POPS)
else
  LIBRARIES := $(UTILS) $(UTILS_QHG) $(CORE) $(ACTIONS) $(GEOINFO) $(ICOSA) $(IO) $(GENES) $(POPS) $(DYNPOPS) 
endif

MAINAPP   := app

export CFLAGS=
export ARFL=-rv
export WARNINGS=-Wall

ifdef AGSTO
  export CADD=$(CADD1) -DAGSTO
else
  export CADD=$(CADD1)
endif

ifdef PROF
  export CADD=-pg -Wall
else
  export CADD=-Wall
endif 

#optimization flags: O1, O2, O3 worse performance
#export CFLAGS=-fomit-frame-pointer -march=pentium4 -dH
export MPI=_MPI_
#export DEFS=
#export DEFS=-DSTRONGLOG 
export __USE_ISOC99=1

.PHONY: opti
.PHONY: debi


opti: export CFLAGS=-O3 $(STD11)
opti: all

debi: export CFLAGS=-g -Wall $(STD11)
debi: all

.PHONY: all $(MAINAPP) $(LIBRARIES)
all: $(MAINAPP)
	$(POST1)

$(MAINAPP) $(LIBRARIES):
	$(MAKE) --directory=$@

plugs: sos
	$(POST2)

sos:
	$(MAKE) --directory=$(PLUGINS) new; 

$(MAINAPP): $(CONFIGURE) $(LIBRARIES)
$(POPS): $(ACTIONS) $(CORE) $(UTILS) $(UTILS_QHG) $(ICOSA) 
$(ACTIONS):  $(CORE) $(GEOINFO) $(UTILS) $(ICOSA) 
$(CORE):  $(GEOINFO) $(UTILS) $(UTILS_QHG) $(ICOSA) 
$(ICOSA): $(UTILS)
$(UTILS_QHG): $(UTILS)
$(IO): $(UTILS) $(QMAPS) $(CORE) 
$(GENES): $(UTILS) $(CORE)

new: clean 
	$(MAKE) all

newo: clean
	$(MAKE) opti

newd: clean
	$(MAKE) debi

plugins:
	$(MAKE) plugs; 

clean:
	@ echo "--- cleaning ---"; \
	for d in $(MAINAPP) $(LIBRARIES) $(VISUAL); \
        do                               \
          $(MAKE) --directory=$$d clean; \
        done

cleanTemp:
	@ echo "--- cleaning ---"; \
	for d in $(LIBRARIES) $(VISUAL); \
	do                               \
	  $(MAKE) --directory=$$d clean; \
	done                             
	$(MAKE) --directory=$(MAINAPP) cleanTemp;

QHGMain: 
	$(MAKE) all && \
	$(MAKE) --directory=$(MAINAPP) QHGMain

QHGMainTemp: 
	$(MAKE) all && \
	$(MAKE) --directory=$(MAINAPP) QHGMainTemp

QHGMain_so: 
	$(MAKE) all && \
	$(MAKE) --directory=$(MAINAPP) QHGMain_so

ifdef CRAY

tools_n:
	$(MAKE) --directory=$(USEFUL) new && \
        $(MAKE) --directory=$(GENES) new;

tools_c:
	$(MAKE) --directory=$(USEFUL) clean && \
        $(MAKE) --directory=$(GENES) clean;

tools:
	$(MAKE) --directory=$(USEFUL) app && \
        $(MAKE) --directory=$(GENES) app;

else

tools_n:
	$(MAKE) --directory=$(VISUAL) new && \
	$(MAKE) --directory=$(TOOLS_ICO) new && \
	$(MAKE) --directory=$(TOOLS_IO) new && \
        $(MAKE) --directory=$(IMPORT) new && \
        $(MAKE) --directory=$(USEFUL) new && \
        $(MAKE) --directory=$(GENES) new;

tools_c:
	$(MAKE) --directory=$(VISUAL) clean && \
	$(MAKE) --directory=$(TOOLS_ICO) clean && \
	$(MAKE) --directory=$(TOOLS_IO) clean && \
        $(MAKE) --directory=$(IMPORT) clean && \
        $(MAKE) --directory=$(USEFUL) clean && \
        $(MAKE) --directory=$(GENES) clean;

tools:
	$(MAKE) --directory=$(VISUAL) all && \
	$(MAKE) --directory=$(TOOLS_ICO) app && \
	$(MAKE) --directory=$(TOOLS_IO) app && \
        $(MAKE) --directory=$(IMPORT) app && \
        $(MAKE) --directory=$(USEFUL) app && \
        $(MAKE) --directory=$(GENES) app;

endif
