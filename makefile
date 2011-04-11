targets=gen read
all: $(targets)
.PHONY: clean
clean:
	rm -f *.~ $(targets)

