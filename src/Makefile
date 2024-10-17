# Makefile for cygwin gcc // ubunutu 
# Copyright (c) 2006, Pieter Blommaert
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

OSTYPE := ${shell uname -s | sed -r "s/^([A-Za-z_]+).*/\1/"} #string part without version indicating the platform (e.g. CYGWIN-NT)
OSTYPE := ${strip ${OSTYPE}}

ifeq ($(OSTYPE),CYGWIN_NT)
PCAP_ROOT := lib/winpcap
PCAP_PATH := ${PCAP_ROOT}/lib
PCAP_INCLUDE := ${PCAP_ROOT}/Include
else
PCAP_ROOT := lib/libpcap
PCAP_PATH := ${PCAP_ROOT}
PCAP_INCLUDE := ${PCAP_ROOT}
endif

EXPAT_ROOT = lib/expat
ZTHREAD_ROOT = lib/zthread
ZTHREAD_LIB = ${ZTHREAD_ROOT}/src/.libs/libZThread.a

PCRE_ROOT := lib/pcre
PCRE_LIB := ${PCRE_ROOT}/.libs/libpcre.a
#PCRE_LIBCPP := ${PCRE_ROOT}/.libs/libpcrecpp.a
PCRE_INCLUDE := ${PCRE_ROOT}

CFLAGS = -I${PCAP_INCLUDE} -I${EXPAT_ROOT}/lib -I${ZTHREAD_ROOT}/include -I${PCRE_INCLUDE} -Wall -fexceptions -fpermissive -DHAVE_EXPAT_CONFIG_H 
ifeq ($(OSTYPE),CYGWIN_NT)
CFLAGS += -mno-cygwin
endif

#LIBS = -L ${PCAP_PATH} -lwpcap -llibexpat 
LIBS= ${EXPAT_ROOT}/.libs/libexpat.a ${ZTHREAD_LIB} ${PCRE_LIB} 
ifeq ($(OSTYPE),CYGWIN_NT)
LIBS += ${PCAP_PATH}/libwpcap.a 
else
LIBS += ${PCAP_PATH}/libpcap.a ${PCAP_PATH}/libpfring.a
endif
#CCC=g++

LINK = ${CXX} ${CFLAGS} 
ifneq ($(OSTYPE),CYGWIN_NT)
LINK += -lpthread
endif

#using special Make variable MAKECMDGOALS to set appropriate flags,...
ifeq ($(MAKECMDGOALS),released)
	CFLAGS+= -O2
	OBJPATH=objs/released
else #debug
ifeq ($(MAKECMDGOALS),compressed)
	CFLAGS+= -O2
	OBJPATH=objs/compressed
else
	CFLAGS+= -ggdb
	OBJPATH=objs/debug
endif
endif


ifeq ($(OSTYPE),CYGWIN_NT)
SVNVERSION=${shell svn info | grep Revision | sed -e "s/Revision: //"}
SVNMODS=${shell svn stat | grep -c "^[MA]"}
TOOLVERSION=\"0.${SVNVERSION}.${SVNMODS}\"
TOOLVERS=0.${SVNVERSION}.${SVNMODS}
else 
ifeq ($(MAKECMDGOALS),released)
SVNVERSION=${shell cat lastReleasedVersion.txt}
TOOLVERSION=\"${OSTYPE}_${SVNVERSION}\"
TOOLVERS=${SVNVERSION}
else
SVNROOT = ${shell svn info | grep "Repository Root" | sed -e "s/Repository Root: //"}
SVNVERSION=${shell svn info | grep Revision | sed -e "s/Revision: //"}
SVNMODS=${shell svn stat | grep -c "^[MA]"}
TOOLVERSION=\"${OSTYPE}_${SVNROOT}_0.${SVNVERSION}.${SVNMODS}\"
TOOLVERS=0.${SVNVERSION}.${SVNMODS}
endif
endif

CPPOBJS := $(patsubst %.cpp,${OBJPATH}/%.o,$(wildcard *.cpp))
COBJS := $(patsubst %.c,${OBJPATH}/%.o,$(wildcard *.c))
PIERFOBJS := ${CPPOBJS} ${COBJS}

PIERFDEPENDS := ${patsubst %.o,%.d,${PIERFOBJS}}

ifeq ($(OSTYPE),CYGWIN_NT)
PIERFEXE := pierf.exe
QUICKEXE := quickCount.exe
else
PIERFEXE := pierf
QUICKEXE := quickCount
endif

define PIERF_MAKE_EXE
${LINK} -g -o ${PIERFEXE} ${PIERFOBJS} ${LIBS}
endef

#### Final targets and clean
${PIERFEXE}: objs/released objs/debug ${PIERFOBJS} objs/released objs/debug
	echo Buildig pierf for ${OSTYPE}.
	${PIERF_MAKE_EXE}

#Gave test sources a different extension to easily differentiate them (could have put them in a separate dir)
threadTest.exe: threadTest.cpt 
	cp threadTest.cpt ${OBJPATH}/threadTest.cpp
	${CXX} ${CFLAGS} -c -o ${OBJPATH}/threadTest.o ${OBJPATH}/threadTest.cpp
	${LINK} -g -o threadTest.exe ${OBJPATH}/threadTest.o ${LIBS}

${QUICKEXE}: ${OBJPATH}/quickCount.o ${OBJPATH}/Counter.o ${OBJPATH}/Var.o ${OBJPATH}/generics.o ${OBJPATH}/generics_pp.o ${OBJPATH}/Exception.o
	echo Building quickCount for ${OSTYPE}.
	${LINK} -g -o ${QUICKEXE} ${OBJPATH}/quickCount.o ${OBJPATH}/Counter.o ${OBJPATH}/Var.o ${OBJPATH}/generics.o ${OBJPATH}/generics_pp.o ${OBJPATH}/Exception.o ${LIBS}
	

objs/released:
	mkdir -p objs/released

objs/debug:
	mkdir -p objs/debug

lib/done:
	${MAKE} -C lib

compressed: ${PIERFEXE}
	@echo Created a compressed pierf executable for ${OSTYPE}

#released will always implicitly to a clean
released: commit clean ${PIERFEXE} count
	@echo RELEASED VERSION CREATED
	rm -f pierf.zip
	echo ${TOOLVERS} > lastReleasedVersion.txt
	/usr/bin/zip pierf.zip ${PIERFEXE} pierf.html license.txt Readme.txt configs/*
	rm -f pierf_sources.zip
	/usr/bin/zip pierf_sources.zip $(wildcard *.cpp) $(wildcard *.c) $(wildcard *.hpp) $(wildcard *.h) Makefile license.txt lastReleasedVersion.txt pierf.html Readme.txt quickCount.sources/* configs/* lib/Makefile lib/ZThread-2.3.2.patch $(wildcard configs/*)
ifeq ($(OSTYPE),CYGWIN_NT)
	svn commit -m "Update pierf.zip/pierf_sources.zip for released version"
endif
	cp pierf.zip pierf_${TOOLVERS}_${OSTYPE}.zip
	cp pierf_sources.zip pierf_sources_${TOOLVERS}.zip
	@echo ZIPFILES UPDATED

#separate, because it must be done before pierf.exe, so cannot put it in the released rule itself
#when building pierf.exe, at compilation of pierf.o, the SVNVERSION is used for the first time
#and make only calculates the values of these variables when needed...
#the svn updated is necessary to bring to the latest revision
commit:
ifeq ($(OSTYPE),CYGWIN_NT)
	svn commit
	svn update 
endif

count:
	lib/c_count/c_count *.cpp

.PHONY: clean

clean:
	rm -f ${PIERFOBJS} ${PIERFDEPENDS} ${PIERFEXE}

#### Rules for the object files

#The target OBJPATH must be in the rule's target, otherwise the % will match dirname/target
#And then make will look for dirname/filename.c, which doesnt exist because c files are
#in another place

${OBJPATH}/pierf.o : pierf.cpp
	${CXX} -DVERSION=${TOOLVERSION} ${CFLAGS} -c -o ${OBJPATH}/pierf.o pierf.cpp

${OBJPATH}/%.o : %.c;
	${CC} -I/usr/include ${CFLAGS} -c -o ${OBJPATH}/$*.o $<

${OBJPATH}/%.o : %.cpp;
	${CXX} ${CFLAGS} -c -o ${OBJPATH}/$*.o $<

${OBJPATH}/quickCount.o : quickCount.sources/quickCount.cpp;
	${CXX} ${CFLAGS} -c -o ${OBJPATH}/quickCount.o quickCount.sources/quickCount.cpp

#From the gnu manual chapter 4.14: Automatic dependency list generation from the source (include statements)
#Must be on one line for the temp filename $$ to be identical
#Dirty trick: .c file is made first as part of include dependency, so putting mkdir -p objs/released here to create at least the released dir automatically since no other system works
${OBJPATH}/%.d: %.c
	mkdir -p objs/released; set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

${OBJPATH}/%.d: %.cpp
	set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

####include the generated dependency Makefiles

include ${PIERFDEPENDS}

