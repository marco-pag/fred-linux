bin = fred_server
srcs = $(wildcard *.c) $(wildcard **/*.c)
objs = $(srcs:.c=.o)
deps = $(objs:.o=.d)

LDFLAGS +=
CFLAGS += -std=gnu99 -Wall -O2
CFLAGS += -D LOG_GLOBAL_LEVEL=LOG_LEV_FULL

$(bin): $(objs)
	$(CC) -o $@ $^ $(LDFLAGS)

# Build header includes dependencies using c preprocessor
%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@

.PHONY: clean
clean:
	rm -f $(objs)
	rm -f $(deps)

