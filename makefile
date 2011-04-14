targets=gen read
flags=-O3
all: $(targets)
.PHONY: clean
clean:
	rm -f *.~ $(targets) *.o

read: read.cpp tvUtil.cpp
	mpic++ -o $@ $^ $(flags)

gen: gen.cpp
	mpic++ -o $@ $^ -llustreapi $(flags)
