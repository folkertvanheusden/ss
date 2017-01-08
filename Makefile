# (C) 2017 by folkert van heusden
# released under AGPL v3.0

VERSION=0.2

DEBUG=-g # -D_DEBUG=1

CXXFLAGS+=-Wall -O3 -ffast-math -DVERSION=\"$(VERSION)\" $(DEBUG)
LDFLAGS+=-lX11 -lXpm $(DEBUG)

OBJS1=st.o
OBJS2=nee-tante-julia.o

DESTPATH=$(PREFIX)/usr/lib/xscreensaver

all: nee-tante-julia test

test: $(OBJS1)
	$(CXX) -Wall -W $(OBJS1) $(LDFLAGS) -o test

nee-tante-julia: $(OBJS2)
	$(CXX) -Wall -W $(OBJS2) $(LDFLAGS) -o nee-tante-julia

install: test nee-tante-julia
	cp test $(DESTPATH)
	cp nee-tante-julia $(DESTPATH)

clean:
	rm -f $(OBJS1) $(OBJS2) test nee-tante-julia

package: clean
	mkdir fvhss-$(VERSION)
	cp *.c* Makefile readme.txt fvhss-$(VERSION)
	tar czf fvhss-$(VERSION).tgz fvhss-$(VERSION)
	rm -rf fvhss-$(VERSION)

check:
	cppcheck -v --enable=all --std=c++11 --inconclusive -I. . 2> err.txt
