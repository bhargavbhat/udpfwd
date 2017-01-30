GCC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -O3
STRIP = strip -s
TARGET = udpfwd

all: udpfwd

udpfwd: udpfwd.c
	$(GCC) $(CFLAGS) udpfwd.c -o $(TARGET)
	$(STRIP) $(TARGET)

clean:
	@rm -f udpfwd *.a *.o

test:
	../test/test_fwd.sh
