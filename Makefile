CC=gcc
CFLAGS=-c -Wall -Wextra -Wpedantic -std=c11
LFLAGS=-Wall

SOURCES=$(wildcard *.c)
INCLUDES=$(wildcard *.h)

OBJDIR := obj
OBJECTS := $(SOURCES:.c=.o)
EXECUTABLE=aliasme

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@
	@grep $(EXECUTABLE) .gitignore || echo $(EXECUTABLE) >> .gitignore

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

install: $(EXECUTABLE)
	@mv aliasme $(HOME)/bin/

clean:
	rm -rf *.o $(EXECUTABLE)
