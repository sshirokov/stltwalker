OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
CFLAGS = -g -O3 $(INCLUDE) $(OPTFLAGS) -Wall
LIBS = $(OPTLIBS) -lm -lpthread -lcurl $(LUA_LIB)

$(OBJS): $(SOURCES)
