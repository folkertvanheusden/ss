# (C) 2017 by folkert van heusden
# released under AGPL v3.0

VERSION=0.3

DEBUG=-ggdb3 # -D_DEBUG=1

CXXFLAGS+=-Wall -Ofast -DVERSION=\"$(VERSION)\" $(DEBUG)
LDFLAGS+=-lX11 -lXpm $(DEBUG)

OBJS1=henon.o
OBJS2=nee-tante-julia.o
OBJS3=bifu.o
OBJS4=lorenzo.o

DESTPATH=$(PREFIX)/usr/lib/xscreensaver

all: nee-tante-julia bifu henon lorenzo

henon: $(OBJS1)
	$(CXX) -Wall -W $(OBJS1) $(LDFLAGS) -o henon

nee-tante-julia: $(OBJS2)
	$(CXX) -Wall -W $(OBJS2) $(LDFLAGS) -o nee-tante-julia

bifu: $(OBJS3)
	$(CXX) -Wall -W $(OBJS3) $(LDFLAGS) -o bifu

lorenzo: $(OBJS4)
	$(CXX) -Wall -W $(OBJS4) $(LDFLAGS) -o lorenzo

install: nee-tante-julia bifu henon lorenzo
	cp bifu nee-tante-julia henon lorenzo $(DESTPATH)

clean:
	rm -f $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) nee-tante-julia bifu henon lorenzo

package: clean
	mkdir fvhss-$(VERSION)
	cp *.c* Makefile readme.txt fvhss-$(VERSION)
	tar czf fvhss-$(VERSION).tgz fvhss-$(VERSION)
	rm -rf fvhss-$(VERSION)

check:
	cppcheck -v --enable=all --std=c++11 --inconclusive -I. . 2> err.txt
