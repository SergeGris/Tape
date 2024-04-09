
SOURCES = \
        src/main.cc \
        src/configuration.cc

CFLAGS=-Wall -Wextra -O2

all:
	g++ $(CFLAGS) $(SOURCES) -o tape-sort
