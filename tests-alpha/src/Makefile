CC = cc -O5 -fno-shared

all: anagram test-printf test-fmath test-math test-llong test-lswlr test-dl2

test: test-dl2.c
	$(CC) -o test-dl2 test-dl2.c

anagram: anagram.c
	$(CC) -o anagram anagram.c

test-printf: test-printf.c
	$(CC) -o test-printf test-printf.c -lm

test-fmath: test-fmath.c
	$(CC) -o test-fmath test-fmath.c -lm

test-math: test-math.c
	$(CC) -o test-math test-math.c -lm

test-llong: test-llong.c
	$(CC) -o test-llong test-llong.c

test-lswlr: test-lswlr.c
	$(CC) -o test-lswlr test-lswlr.c

test:	all
	../simplesim-0.1/sim-safe anagram words < input.txt
	../simplesim-0.1/sim-safe test-printf
	../simplesim-0.1/sim-safe test-fmath
	../simplesim-0.1/sim-safe test-math
	../simplesim-0.1/sim-safe test-llong
	../simplesim-0.1/sim-safe test-lswlr

distclean:
	-make clean

clean:
	rm -f anagram test-printf test-fmath test-math test-llong test-lswlr test-as *.[oia] core *~

