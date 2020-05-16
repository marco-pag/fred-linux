bin = fred_server
srcs = $(wildcard *.c) $(wildcard **/*.c)
objs = $(srcs:.c=.o)
deps = $(objs:.o=.d)

LDFLAGS +=
CFLAGS += -std=gnu99 -Wall -g #-O2
CFLAGS += -D LOG_GLOBAL_LEVEL=LOG_LEV_FULL

$(bin): $(objs)
	$(CC) -o $@ $^ $(LDFLAGS)

# include all dep
-include $(deps)

# Build header includes dependencies using c preprocessor
%.d: %.c
	$(CPP) $< -MM -MT $(@:.d=.o) > $@

.PHONY: clean
clean:
	rm -f $(bin) $(objs) $(deps)

