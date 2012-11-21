OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
CFLAGS = -g -O2 $(INCLUDE) $(OPTFLAGS) -Wall -Wno-incompatible-pointer-types
LIBS = $(OPTLIBS) -lm -lpthread -lcurl $(LUA_LIB)

$(OBJS): $(SOURCES)
