targets=gen read
flags=-O3
all: $(targets)
.PHONY: clean
clean:
	rm -f *.~ $(targets) *.o

read: read.cpp tvUtil.cpp
	mpicxx -o $@ $^ $(flags)

gen: gen.cpp
	mpicxx -o $@ $^ -llustreapi $(flags)
