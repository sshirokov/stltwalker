#-*- mode:makefile-gmake; -*-
## Env Setup
ROOT = $(shell pwd)
TARGET = stltwalker

## DRY Makefile peices
include make/init.mk

## Target specifics
SOURCES += src/$(TARGET).c

## Compute the final versions of $(OBJS), $(CFLAGS), $(LIBS) and $(CPPFLAGS)
include make/commit.mk

## Porcelain targets
all: $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET) $(TARGET).new

.PHONY: all clean

## Plumbing
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@.new $^ $(LIBS)
	mv $@.new $@


## Set the default goal reasonabley
.DEFAULT_GOAL = all
