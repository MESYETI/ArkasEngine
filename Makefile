SOURCES := $(wildcard source/*.c) $(wildcard source/**/*.c)
OBJECTS := $(patsubst source/%.c,bin/%.o,$(SOURCES))
OUT     := arkas

ifeq ($(PLAT),windows)
	CC := x86_64-w64-mingw32-gcc
	override LDLIBS := $(LDLIBS) $(shell x86_64-w64-mingw32-pkg-config sdl2 --libs) -lopengl32
	override CFLAGS := $(CFLAGS) $(shell x86_64-w64-mingw32-pkg-config sdl2 --cflags)
else
	override LDLIBS += -lSDL2 -lGL
endif
LD := $(CC)

override CFLAGS += -std=c99 -Wall -Wextra -Wuninitialized -Wundef -pedantic -Ilib
override LDLIBS += -lm

ifeq ($(BUILD),release)
	override CFLAGS += -O3
else
	override CFLAGS += -g
	ifeq ($(ASAN),y)
		override CFLAGS += -fsanitize=address
	endif
endif

.SECONDEXPANSION:

deps.filter := %.c %.h
deps.option := -MM
define deps
$$(filter $$(deps.filter),,$$(shell $(CC) $(CFLAGS) $(CPPFLAGS) -E $(deps.option) $(1)))
endef

all: $(OUT)
	@:

run: $(OUT)
	'$(dir $<)$(notdir $<)' $(RUNFLAGS)

$(OUT): $(OBJECTS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/:
	mkdir -p bin

bin/backends:
	mkdir -p bin/backends

bin/%.o: source/%.c $(call deps,source/%.c) | bin/ bin/backends
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

clean:
	rm -r bin

distclean: clean
	rm $(OUT)

.PHONY: all run clean distclean
