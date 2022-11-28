CC=gcc
EXTRAFLAGS=
CFLAGS=-c -Wall -Wextra -Wpedantic -std=c11 $(EXTRAFLAGS)
LFLAGS=-Wall

SOURCES=$(wildcard *.c) $(wildcard src/*.c)
INCLUDES=$(wildcard *.h)

OBJDIR := obj
OBJECTS := $(SOURCES:.c=.o)
EXECUTABLE=aliasme

.PHONY: $(SOURCES)
.PHONY: all
all: $(EXECUTABLE)

.PHONY: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@
	@grep $(EXECUTABLE) .gitignore || echo $(EXECUTABLE) >> .gitignore

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

install: $(EXECUTABLE)
	@mv aliasme $(HOME)/bin/
	@cp completions/aliasme.fish $(HOME)/.config/fish/completions

clean:
	rm -rf *.o src/*.o $(EXECUTABLE)
