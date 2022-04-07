## SW130514 This is a copy of MiniSAT's 'mtl/template.mk' in which all occurences of '.o' have been 
## replaced by '.$(OBJEXT)'
##
##

##
##  Template makefile for Standard, Profile, Debug, Release, and Release-static versions
##
##    eg: "make rs" for a statically linked release version.
##        "make d"  for a debug version (no optimizations).
##        "make"    for the standard version (optimized, but with debug information and assertions active)

PWD        = $(shell pwd)
EXEC      ?= $(notdir $(PWD))

CSRCS      = $(wildcard $(PWD)/*.cc) 
DSRCS      = $(foreach dir, $(DEPDIR), $(filter-out $(MROOT)/$(dir)/Main.cc, $(wildcard $(MROOT)/$(dir)/*.cc)))
CHDRS      = $(wildcard $(PWD)/*.h)

COBJS      = $(CSRCS:.cc=.$(OBJEXT)) $(DSRCS:.cc=.$(OBJEXT))

PCOBJS     = $(addsuffix p,  $(COBJS))
DCOBJS     = $(addsuffix d,  $(COBJS))
RCOBJS     = $(addsuffix r,  $(COBJS))

UNAME                           = $(shell uname -a)
CRAY_PLATFORM_NAME              = nakuru

ifeq (${CRAY_PLATFORM_NAME}, $(findstring ${CRAY_PLATFORM_NAME}, ${UNAME}))
        #Variables for Nakuru Cray system
        MPIXX                    = CC
        CXX                      = CC
        CFLAGS                  += -Wall -Wno-parentheses -I${MROOT} -I../ -I/sw/UNCLASSIFIED/glog-master/include \
                                   -I/sw/UNCLASSIFIED/cudd-release/include -I/sw/UNCLASSIFIED/zlib-1.2.11/include \
                                   -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS
        LFLAGS                  += -Wall -L/sw/UNCLASSIFIED/glog-master/lib -L/sw/UNCLASSIFIED/zlib-1.2.11/lib -lz -lglog
        COMPTIMIZE              ?= -O3
else
        #Default variables
        MPIXX      = mpicxx -c # Marshall Edit
        CXX       ?= mpicxx
        CFLAGS    ?= -Wall -Wno-parentheses
        LFLAGS    ?= -Wall

        COPTIMIZE ?= -O3 # -enable-libstdcxx-debug

        CFLAGS    += -I$(MROOT) -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS
        LFLAGS    += -lz
endif

.PHONY : s p d r rs clean 

s:	$(EXEC)
p:	$(EXEC)_profile
d:	$(EXEC)_debug
r:	$(EXEC)_release
rs:	$(EXEC)_static

libs:	lib$(LIB)_standard.a
libp:	lib$(LIB)_profile.a
libd:	lib$(LIB)_debug.a
libr:	lib$(LIB)_release.a

## Compile options
%.$(OBJEXT):			CFLAGS +=$(COPTIMIZE) -ggdb -D DEBUG
%.$(OBJEXT)p:			CFLAGS +=$(COPTIMIZE) -pg -ggdb -D NDEBUG
%.$(OBJEXT)d:			CFLAGS +=-O0 -ggdb -D DEBUG
%.$(OBJEXT)r:			CFLAGS +=$(COPTIMIZE) -ggdb -D NDEBUG

## Link options
$(EXEC):		LFLAGS += -ggdb
$(EXEC)_profile:	LFLAGS += -ggdb -pg
$(EXEC)_debug:		LFLAGS += -ggdb
#$(EXEC)_release:	LFLAGS += ...
$(EXEC)_static:		LFLAGS += --static

## Dependencies
$(EXEC):		$(COBJS)
$(EXEC)_profile:	$(PCOBJS)
$(EXEC)_debug:		$(DCOBJS)
$(EXEC)_release:	$(RCOBJS)
$(EXEC)_static:		$(RCOBJS)

lib$(LIB)_standard.a:	$(filter-out */Main.$(OBJEXT),  $(COBJS))
lib$(LIB)_profile.a:	$(filter-out */Main.$(OBJEXT)p, $(PCOBJS))
lib$(LIB)_debug.a:	$(filter-out */Main.$(OBJEXT)d, $(DCOBJS))
lib$(LIB)_release.a:	$(filter-out */Main.$(OBJEXT)r, $(RCOBJS))


## Build rule
%.$(OBJEXT) %.$(OBJEXT)p %.$(OBJEXT)d %.$(OBJEXT)r:	%.cc
	@echo Compiling: $(subst $(MROOT)/,,$@)
	@echo COMMAND: $(MPIXX) $(CFLAGS) -o $@ $<
	@$(MPIXX) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

## Linking rules (standard/profile/debug/release)
$(EXEC) $(EXEC)_profile $(EXEC)_debug $(EXEC)_release $(EXEC)_static:
	@echo Linking: "$@ ( $(foreach f,$^,$(subst $(MROOT)/,,$f)) )"
ifeq (${CRAY_PLATFORM_NAME}, $(findstring ${CRAY_PLATFORM_NAME}, ${UNAME}))
	@echo DJMCKEN: Skipping linking and strengthener executable build - made cray v.unhappy
else
	@$(MPIXX) $^ $(LFLAGS) -o $@
endif

## Library rules (standard/profile/debug/release)
lib$(LIB)_standard.a lib$(LIB)_profile.a lib$(LIB)_release.a lib$(LIB)_debug.a:
	@echo Making library: "$@ ( $(foreach f,$^,$(subst $(MROOT)/,,$f)) )"
	@$(AR) -rcsv $@ $^

## Library Soft Link rule:
libs libp libd libr:
	@echo "Making Soft Link: $^ -> lib$(LIB).a"
	@ln -sf $^ lib$(LIB).a

## Clean rule
clean:
	@rm -f $(EXEC) $(EXEC)_profile $(EXEC)_debug $(EXEC)_release $(EXEC)_static \
	  $(COBJS) $(PCOBJS) $(DCOBJS) $(RCOBJS) *.core depend.mk
	@rm -f MpiBuffer.h.gch
	@rm -f ../minisat/*/*.o*

## Make dependencies
depend.mk: $(CSRCS) $(CHDRS)
	@echo Making dependencies
	@$(MPIXX) $(CFLAGS) -I$(MROOT) \
	   $(CSRCS) -MM | sed 's|\(.*\):|$(PWD)/\1 $(PWD)/\1r $(PWD)/\1d $(PWD)/\1p:|' > depend.mk
	@for dir in $(DEPDIR); do \
	      if [ -r $(MROOT)/$${dir}/depend.mk ]; then \
		  echo Depends on: $${dir}; \
		  cat $(MROOT)/$${dir}/depend.mk >> depend.mk; \
	      fi; \
	  done

-include $(MROOT)/mtl/config.mk
-include depend.mk
