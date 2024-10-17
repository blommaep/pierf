# Makefile for cygwin gcc
# Copyright (c) 2006, Pieter Blommaert
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# The names of the author and contributors may be used to endorse or promote products derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# For license of the packages included here, see inside the packages
# This Makefile was created for cygwin/mingw platform. The pcap library used is the windows one. Other platforms should be possible but currently not built...

CFLAGS := -fpermissive 
OSTYPE := ${shell uname -s | sed -r "s/^([A-Za-z_]+).*/\1/"} #string part without version indicating the platform (e.g. CYGWIN-NT)
OSTYPE := ${strip ${OSTYPE}}

EXPATPKG := $(wildcard expat*.tar.gz)
EXPATPK2 := $(patsubst %.tar.gz,%2.tar.gz,$(EXPATPKG))
EXPATTAR := $(patsubst %.tar.gz,%.tar,$(EXPATPK2))
EXPATDIR := $(patsubst %.tar.gz,%,$(EXPATPKG))

WINPCAPPKG := $(wildcard WpdPack*.zip)

LIBPCAPPKG := $(wildcard libpcap-*.tar.gz)
LIBPCAPPK2 := $(patsubst %.tar.gz,%2.tar.gz,$(LIBPCAPPKG))
LIBPCAPTAR := $(patsubst %.tar.gz,%.tar,$(LIBPCAPPK2))
LIBPCAPDIR := $(patsubst %.tar.gz,%,$(LIBPCAPPKG))
  
ZTHREADPKG := $(wildcard ZThread-*.tar.gz)
ZTHREADPK2 := $(patsubst %.tar.gz,%2.tar.gz,$(ZTHREADPKG))
ZTHREADTAR := $(patsubst %.tar.gz,%.tar,$(ZTHREADPK2))
ZTHREADDIR := $(patsubst %.tar.gz,%,$(ZTHREADPKG))

PCREPKG := $(wildcard pcre-*.tar.gz)
PCREPK2 := $(patsubst %.tar.gz,%2.tar.gz,$(PCREPKG))
PCRETAR:= $(patsubst %.tar.gz,%.tar,$(PCREPK2))
PCREDIR := $(patsubst %.tar.gz,%,$(PCREPKG))

CCOUNTPKG  := $(wildcard c_count-*.tar.gz)
CCOUNTDIR  := $(patsubst %.tar.gz,%,$(CCOUNTPKG))

#PTHREADPKG := $(wildcard pthreads-w32-*-release.tar.gz)
#PTHREADPK2 := $(patsubst %.tar.gz,%2.tar.gz,$(PTHREADPKG))
#PTHREADTAR := $(patsubst %.tar.gz,%.tar,$(PTHREADPK2))
#PTHREADDIR := $(patsubst %.tar.gz,%,$(PTHREADPKG))

ifeq ($(OSTYPE),CYGWIN_NT)
	MKLIBS = expat/.libs/libexpat.a winpcap/lib/libwpcap.a zthread/src/.libs/libZThread.a c_count/c_count.exe pcre/.libs/libpcre.a
else
	MKLIBS = expat/.libs/libexpat.a zthread/src/.libs/libZThread.a libpcap/libpcap.a c_count/c_count pcre/.libs/libpcre.a 
endif

ifeq ($(OSTYPE),CYGWIN_NT)
	CFLAGS_PCRE = "-O2 -mno-cygwin -Wall"
else
	CFLAGS_PCRE = "-O2"
endif


#### Final targets and clean
.PHONY: all

all: $(MKLIBS)
	
expat/.libs/libexpat.a: expat/Makefile
	${MAKE} -C expat

expat/Makefile:
	echo Expat libpcap for $(OSTYPE)
	cp ${EXPATPKG} ${EXPATPK2}
	gzip -d ${EXPATPK2}
	tar -xvf ${EXPATTAR}
	rm -f ${EXPATTAR}
	mv ${EXPATDIR} expat
	cd expat && ./configure
ifeq ($(OSTYPE),CYGWIN_NT)
	cd expat && cat Makefile | sed -e 's/CFLAGS = /CFLAGS = -mno-cygwin /' > Makefile.new
	mv -f expat/Makefile.new expat/Makefile
endif

winpcap/lib/libwpcap.a: 
	echo Preparing libpcap for $(OSTYPE)
	unzip -o ${WINPCAPPKG}
	mv WpdPack winpcap


libpcap/Makefile:
	echo Building libpcap for $(OSTYPE)
	cp ${LIBPCAPPKG} ${LIBPCAPPK2}
	gzip -d ${LIBPCAPPK2}
	tar -xvf ${LIBPCAPTAR}
	rm -f ${LIBPCAPTAR}
	mv ${LIBPCAPDIR} libpcap
	cd libpcap && ./configure 

libpcap/libpcap.a: libpcap/Makefile
	${MAKE} -C libpcap

zthread/Makefile:
	echo Building libpcap for $(OSTYPE)
	cp ${ZTHREADPKG} ${ZTHREADPK2}
	gzip -d ${ZTHREADPK2}
	tar -xvf ${ZTHREADTAR}
	rm -f ${ZTHREADTAR}
	mv ${ZTHREADDIR} zthread
ifeq ($(OSTYPE),CYGWIN_NT)
	cd zthread && ./configure --host=mingw32  --enable-win32
	cd zthread && cat Makefile | sed -e 's/CFLAGS = /CFLAGS = -mno-cygwin /' > Makefile.new
	mv -f zthread/Makefile.new zthread/Makefile
	cd zthread/src && cat Makefile | sed -e 's/CFLAGS = /CFLAGS = -mno-cygwin /' > Makefile.new
	mv -f zthread/src/Makefile.new zthread/src/Makefile
else
	cd zthread && ./configure
endif
	cp ZThread-2.3.2.patch zthread
	cd zthread && patch -p1 < ZThread-2.3.2.patch

zthread/src/.libs/libZThread.a: zthread/Makefile
	echo Building zthread for $(OSTYPE)
	${MAKE} -C zthread

pcre/Makefile:
	echo Configuring pcre for $(OSTYPE)
	cp ${PCREPKG} ${PCREPK2}
	gzip -d ${PCREPK2}
	tar -xvf ${PCRETAR}
	rm -f ${PCRETAR}
	mv ${PCREDIR} pcre
	cd pcre && CFLAGS=${CFLAGS_PCRE} ./configure --disable-cpp --disable-shared --enable-newline-is-anycrlf --enable-utf8 --enable-unicode-properties

pcre/.libs/libpcre.a: pcre/Makefile
	echo Building libpcre - perl compatible regex
	${MAKE} -C pcre

c_count/makefile:
	cp ${CCOUNTPKG} c_count.tar.gz
	gunzip c_count.tar.gz
	tar -xvf c_count.tar
	rm c_count.tar
	mv ${CCOUNTDIR} c_count
	cd c_count && dos2unix *.c *.h configure *.sh
	cd c_count && chmod +x configure *.sh
	cd c_count && ./configure

c_count/c_count.exe: c_count/makefile
	echo Building c_count for $(OSTYPE)
	${MAKE} -C c_count

c_count/c_count: c_count/makefile
	echo Building c_count for $(OSTYPE)
	${MAKE} -C c_count

# pthread is included in winpcap
#pthread/Makefile:
#	cp ${PTHREADPKG} ${PTHREADPK2}
#	gzip -d ${PTHREADPK2}
#	tar -xvf ${PTHREADTAR}
#	rm -f ${PTHREADTAR}
#	mv ${PTHREADDIR} pthread
#	cp pthread.patch pthread
#	cd pthread && patch < pthread.patch

.PHONY: clean

clean:
	rm -rf expat
	rm -rf winpcap
	rm -rf c_count

