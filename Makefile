# (C) 2017 by folkert van heusden
# released under AGPL v3.0

VERSION=0.2

DEBUG=-ggdb3 # -D_DEBUG=1

CXXFLAGS+=-Wall -O3 -ffast-math -DVERSION=\"$(VERSION)\" $(DEBUG)
LDFLAGS+=-lX11 -lXpm $(DEBUG)

OBJS1=st.o
OBJS2=nee-tante-julia.o
OBJS3=bifu.o

DESTPATH=$(PREFIX)/usr/lib/xscreensaver

all: nee-tante-julia test bifu

test: $(OBJS1)
	$(CXX) -Wall -W $(LDFLAGS) $(OBJS1) -lmosquitto -o test

nee-tante-julia: $(OBJS2)
	$(CXX) -Wall -W $(LDFLAGS) $(OBJS2) -o nee-tante-julia

bifu: $(OBJS3)
	$(CXX) -Wall -W $(LDFLAGS) $(OBJS3) -o bifu

install: test nee-tante-julia
	cp test $(DESTPATH)
	cp nee-tante-julia $(DESTPATH)

clean:
	rm -f $(OBJS1) $(OBJS2) $(OBJS3) test nee-tante-julia bifu

package: clean
	mkdir fvhss-$(VERSION)
	cp *.c* Makefile readme.txt fvhss-$(VERSION)
	tar czf fvhss-$(VERSION).tgz fvhss-$(VERSION)
	rm -rf fvhss-$(VERSION)

check:
	cppcheck -v --enable=all --std=c++11 --inconclusive -I. . 2> err.txt
