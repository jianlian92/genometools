#
# Copyright (c) 2006-2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
# Copyright (c) 2006-2007 Center for Bioinformatics, University of Hamburg 
# See LICENSE file or http://genometools.org/license.html for license details
#

CC:=gcc
LD:=gcc
INCLUDEOPT:= -I$(CURDIR)/src -I$(CURDIR)/obj -I$(CURDIR)/src/lua-5.1.1/src \
             -I$(CURDIR)/src/expat-2.0.0/lib
CFLAGS:=
GT_CFLAGS:= -Wall $(INCLUDEOPT)
LDFLAGS:=
LDLIBS:=-lm -lz

LIBGT_SRC:=$(notdir $(wildcard src/libgt/*.c))
LIBGT_OBJ:=$(LIBGT_SRC:%.c=obj/%.o)

TOOLS_SRC:=$(notdir $(wildcard src/tools/*.c))
TOOLS_OBJ:=$(TOOLS_SRC:%.c=obj/%.o)

LIBEXPAT_OBJ:=obj/xmlparse.o obj/xmlrole.o obj/xmltok.o

LIBLUA_OBJ=obj/lapi.o obj/lcode.o obj/ldebug.o obj/ldo.o obj/ldump.o \
           obj/lfunc.o obj/lgc.o obj/llex.o obj/lmem.o obj/lobject.o \
           obj/lopcodes.o obj/lparser.o obj/lstate.o obj/lstring.o   \
           obj/ltable.o obj/ltm.o obj/lundump.o obj/lvm.o obj/lzio.o \
           obj/lauxlib.o obj/lbaselib.o obj/ldblib.o obj/liolib.o    \
           obj/lmathlib.o obj/loslib.o obj/ltablib.o obj/lstrlib.o   \
           obj/loadlib.o obj/linit.o

LIBRNV_OBJ := obj/rn.o obj/rnc.o obj/rnd.o obj/rnl.o obj/rnv.o obj/rnx.o obj/drv.o  \
              obj/ary.o obj/xsd.o obj/xsd_tm.o obj/dxl.o obj/dsl.o obj/sc.o obj/u.o \
              obj/ht.o obj/er.o obj/xmlc.o obj/s.o obj/m.o obj/rx.o

# process arguments
ifeq ($(opt),no)
  GT_CFLAGS += -g
else
  GT_CFLAGS += -Os
endif

ifeq ($(assert),no)
  GT_CFLAGS += -DNDEBUG
endif

# set prefix for install target
prefix ?= /usr/local

all: dirs lib/libgt.a bin/gt bin/rnv

dirs:
	@test -d obj     || mkdir -p obj 
	@test -d lib     || mkdir -p lib 
	@test -d bin     || mkdir -p bin 

lib/libexpat.a: $(LIBEXPAT_OBJ)
	ar ruv $@ $(LIBEXPAT_OBJ)
ifdef RANLIB
	$(RANLIB) $@
endif

lib/libgt.a: obj/gt_build.h obj/gt_cc.h obj/gt_cflags.h obj/gt_version.h \
             $(LIBGT_OBJ) $(LIBLUA_OBJ)
	ar ruv $@ $(LIBGT_OBJ) $(LIBLUA_OBJ)
ifdef RANLIB
	$(RANLIB) $@
endif

lib/librnv.a: $(LIBRNV_OBJ)
	ar ruv $@ $(LIBRNV_OBJ)
ifdef RANLIB
	$(RANLIB) $@
endif

bin/gt: obj/gt.o obj/gtr.o $(TOOLS_OBJ) lib/libgt.a
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/rnv: obj/xcl.o lib/librnv.a lib/libexpat.a
	$(LD) $(LDFLAGS) $^ -o $@

obj/gt_build.h:
	@date +'#define GT_BUILT "%Y-%m-%d %H:%M:%S"' > $@

obj/gt_cc.h:
	@echo '#define GT_CC "'`$(CC) --version | head -n 1`\" > $@

obj/gt_cflags.h:
	@echo '#define GT_CFLAGS "$(CFLAGS) $(GT_CFLAGS)"' > $@

obj/gt_version.h: VERSION
	@echo '#define GT_VERSION "'`cat VERSION`\" > $@

# we create the dependency files on the fly
obj/%.o: src/%.c
	$(CC) -c $< -o $@  $(CFLAGS) $(GT_CFLAGS) -MT $@ -MMD -MP -MF $(@:.o=.d)

obj/%.o: src/libgt/%.c
	$(CC) -c $< -o $@  $(CFLAGS) $(GT_CFLAGS) -MT $@ -MMD -MP -MF $(@:.o=.d)

obj/%.o: src/tools/%.c
	$(CC) -c $< -o $@  $(CFLAGS) $(GT_CFLAGS) -MT $@ -MMD -MP -MF $(@:.o=.d)

obj/%.o: src/expat-2.0.0/lib/%.c
	$(CC) -c $< -o $@  $(CFLAGS) $(GT_CFLAGS) -DHAVE_MEMMOVE -MT $@ -MMD \
        -MP -MF $(@:.o=.d)

obj/%.o: src/lua-5.1.1/src/%.c
	$(CC) -c $< -o $@  $(CFLAGS) $(GT_CFLAGS) -DLUA_USE_POSIX -MT $@ -MMD \
        -MP -MF $(@:.o=.d)

obj/%.o: src/rnv-1.7.8/%.c
	$(CC) -c $< -o $@  $(CFLAGS) $(GT_CFLAGS) -DUNISTD_H="<unistd.h>" \
        -DEXPAT_H="<expat.h>" -DRNV_VERSION="\"1.7.8\"" -MT $@ -MMD -MP   \
        -MF $(@:.o=.d)

# read deps
-include obj/*.d

.PHONY: dist srcdist release gt libgt install splint test clean cleanup

dist: all
	tar cvzf gt-`cat VERSION`.tar.gz bin/gt_*

srcdist:
	git archive --format=tar --prefix=genometools-`cat VERSION`/ HEAD | \
        gzip -9 > genometools-`cat VERSION`.tar.gz 

release:
	git tag "v`cat VERSION`"
	git archive --format=tar --prefix=genometools-`cat VERSION`/ HEAD | \
        gzip -9 > genometools-`cat VERSION`.tar.gz 

gt: dirs bin/gt

libgt: dirs lib/libexpat.a lib/libgt.a

install:
	test -d $(prefix)/bin || mkdir -p $(prefix)/bin
	cp bin/gt $(prefix)/bin
	cp -r gtdata $(prefix)/bin
	test -d $(prefix)/include/libgt || mkdir -p $(prefix)/include/libgt
	cp src/gt.h $(prefix)/include	
	cp src/libgt/*.h $(prefix)/include/libgt
	test -d $(prefix)/lib || mkdir -p $(prefix)/lib
	cp lib/libgt.a $(prefix)/lib
	
splint:
	splint -f $(CURDIR)/testdata/Splintoptions $(INCLUDEOPT) $(CURDIR)/src/*.c

test: all
	bin/gt -test
	cd testsuite && env -i ruby -I. testsuite.rb -testdata $(CURDIR)/testdata -bin $(CURDIR)/bin

clean:
	rm -rf obj
	rm -rf testsuite/stest_testsuite testsuite/stest_stest_tests

cleanup: clean
	rm -rf lib bin
