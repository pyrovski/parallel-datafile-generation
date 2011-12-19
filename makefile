targets=gen read
flags=-O3

ifeq ($(dbg),1)
flags=-g -O0
endif

# build with "make lustre=1" to use lustre
ifeq ($(lustre),1)
flags+= -llustreapi -DhaveLustre
endif

all: $(targets)
.PHONY: clean
clean:
	rm -f *.~ $(targets) *.o

read: read.cpp tvUtil.cpp
	mpicxx -o $@ $^ $(flags)

gen: gen.cpp
	mpicxx -o $@ $^ $(flags)
