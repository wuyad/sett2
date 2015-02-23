INCDIR+=$(LIBS_ROOT)/ACE_wrappers
LIBSDIR+=$(LIBS_ROOT)/ACE_wrappers/lib
LIBS+=ACE

CXXFLAGS+=-DACE_AS_STATIC_LIBS

# to use ace compiled with gcc normal
# __ACE_INLINE__ must define in "ace/config.h"
