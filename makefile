targets=gen read
all: $(targets)
.PHONY: clean
clean:
	rm -f *.~ $(targets) *.o

read: read.cpp
	mpic++ -o $@ $^ 