targets=gen read
flags=-O3

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
