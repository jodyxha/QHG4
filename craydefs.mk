ifdef CRAY

  export CRAY_DEF=$(CFLAGS) -DCRAY
  ifndef C_STD
    ifdef CRAY_STD
      C_STD := $(CRAY_STD)
    else
      C_STD := $(QHG_STD)
    endif   
  endif

  ifndef COMP
    ifdef CRAY_OMP
      COMP := $(CRAY_OMP)
    else
      COMP := $(QHG_OMP)
    endif
  endif

else

  export CRAY_DEF=
  ifndef C_STD
    ifdef GNU_STD
      C_STD := $(GNU_STD)
    else
      C_STD := $(QHG_STD)
    endif
  endif

  ifndef COMP
    ifdef GNU_OMP
      COMP := $(GNU_OMP)
    else
      COMP := $(QHG_OMP)
    endif
  endif

endif