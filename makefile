targets=gen read
all: $(targets)
.PHONY: clean
clean:
	rm -f *.~ $(targets) *.o

read: read.cpp tvUtil.cpp
	mpic++ -o $@ $^ 

gen: gen.cpp
	mpic++ -o $@ $^