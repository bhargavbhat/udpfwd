GCC = gcc
CFLAGS = -D_POSIX_C_SOURCE=200112L -std=c99 -Wall -Wextra -Werror -Wpedantic -O3
STRIP = strip -s
TARGET = udpfwd

all: udpfwd

udpfwd: udpfwd.c udpfwd.h
	$(GCC) $(CFLAGS) udpfwd.c -o $(TARGET)
	$(STRIP) $(TARGET)

clean:
	@rm -f udpfwd *.a *.o

test:
	../test/test_fwd.sh
