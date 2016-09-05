all: cue-bin-split

cue-bin-split: cue-bin-split.o

test: all
	@./test/run-tests.sh

clean:
	rm -vf cue-bin-split *.o

.PHONY: all clean test
