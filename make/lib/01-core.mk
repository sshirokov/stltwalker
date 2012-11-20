HEADERS = $(shell find $(ROOT) -name '*.h')
INCLUDE += -I$(ROOT)/src
TARGET_SOURCES = $(shell find $(ROOT)/src -name '*.c' -not -name '$(TARGET).c')

SOURCES += $(TARGET_SOURCES)
