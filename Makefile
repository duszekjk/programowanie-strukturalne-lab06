CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O2

EXAMPLES := forward square jump invalid_command

.PHONY: all clean examples

all: examples

examples: $(EXAMPLES:%=build/%)

build:
	mkdir -p build

build/%: examples/%.c include/ue_ipc.h | build
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build
