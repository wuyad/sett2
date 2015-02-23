# commands
CC=g++
AR=ar

PLATFORM=$(shell uname)
ifeq ($(findstring CYGWIN,$(PLATFORM)),CYGWIN)
	PLATFORM=CYGWIN
endif
include ../platform-$(PLATFORM)-$(CC).mak

# input and output dirctory
BINDIR=../../bin
CFGDIR=../../conf
LIBDIR=../../lib
LOGDIR=../../log
OBJDIR=obj

# some temp micro
PROJECTNAME=$(notdir $(shell pwd))

ifeq (LIB,$(PROJECTTYPE))
	OUTFILENAME=$(LIBPREFIX)$(PROJECTNAME)$(LIBPOSTFIX)
	OUTFILE=$(LIBDIR)/$(OUTFILENAME)
else
	ifeq ($(PROJECTTYPE),DLL)
		OUTFILENAME=$(DLLPREFIX)$(PROJECTNAME)$(DLLPOSTFIX)
		OUTFILE=$(LIBDIR)/$(OUTFILENAME)
		CXXFLAGS+= $(DLL_COMPILER_FLAG)
release: CXXFLAGS:=$(CXXFLAGS) $(DLL_COMPILER_FLAG)
	else
		OUTFILENAME=$(PROJECTNAME)
		OUTFILE=$(BINDIR)/$(OUTFILENAME)
	endif
endif

SRCS_CPP=$(filter-out $(EXCLUDE),$(wildcard *.cpp))
SRCS_CC=$(filter-out $(EXCLUDE),$(wildcard *.cc))
SRCS_C=$(filter-out $(EXCLUDE),$(wildcard *.c))

OBJS_CPP=$(SRCS_CPP:%.cpp=$(OBJDIR)/%.o)
OBJS_CC=$(SRCS_CC:%.cc=$(OBJDIR)/%.o)
OBJS_C=$(SRCS_C:%.c=$(OBJDIR)/%.o)

DOBJS_CPP=$(SRCS_CPP:%.cpp=%.d)
DOBJS_CC=$(SRCS_CC:%.cc=%.d)
DOBJS_C=$(SRCS_C:%.c=%.d)

SRCS=$(SRCS_CPP) $(SRCS_CC) $(SRCS_C)
OBJS=$(OBJS_CPP) $(OBJS_CC) $(OBJS_C)
DOBJS=$(DOBJS_CPP) $(DOBJS_CC) $(DOBJS_C)

# compile and link command
COMPILER=$(CC) $(CXXFLAGS) -c -o $@ $(INCDIR:%=-I%) $<

ifeq (LIB,$(PROJECTTYPE))
	LINKER=$(AR) $(ARFLAGS) $@ $?
else
	ifeq ($(PROJECTTYPE),DLL)
		LINKER=$(CC) $(DLL_LINKER_FLAG) -o $(OUTFILE) $(OBJS) $(LIBSDIR:%=-L%) $(LIBS:%=-l%)
	else
		LINKER=$(CC) $(LDFLAGS) -o $(OUTFILE) $(OBJS) $(LIBSDIR:%=-L%) $(LIBS:%=-l%)
	endif
endif

MKDIR=mkdir -p $(LIBDIR) $(BINDIR) $(CFGDIR) $(OBJDIR) $(LOGDIR)
# Build rules
de debug all: $(OUTFILE)
	@echo "Fine."
release: clean $(OUTFILE)
	@echo "Fine."
$(OUTFILE): $(OBJS)
	@echo "Linking $@........................................................"
	$(LINKER)
r run: debug
	$(OUTFILE) $(ARGS)
re rebuild:cl all

define pre_execute
	for a in $(DOBJS); do \
		$(CC) $(DFLAGS) $(INCDIR:%=-I%) `echo $$a | sed 's/\.d/\.cpp/'` | sed 's/\(.*\)\.o[ :]*/$(OBJDIR)\/\1.o: /g' > $$a; \
	done; \
	echo $(DOBJS)
endef

ifeq ($(DEPENDENCYFLAG), 1)
include $(shell $(pre_execute))
endif

# Pattern rules
$(OBJS_CPP): $(OBJDIR)/%.o: %.cpp
	@echo "Compiling $@......................................................"
	@$(MKDIR)
	$(COMPILER)
$(OBJS_CC): $(OBJDIR)/%.o: %.cc
	@echo "Compiling $@......................................................"
	@$(MKDIR)
	$(COMPILER)
$(OBJS_C): $(OBJDIR)/%.o: %.c
	@echo "Compiling $@......................................................"
	@$(MKDIR)
	$(COMPILER)

.PHONY: clean cl up update ci checkin commit doc test re rebuild
# Clean this project
cl clean:
	@rm -fr core
	@rm -fr $(DOBJS)
	@rm -fr $(OBJDIR)
	@rm -fr $(OUTFILE)
	
up update:
	@cvs update

ci checkin commit:
	@cvs ci
doc:

test:
	echo $(INCDIR)
