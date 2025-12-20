SOURCES := $(wildcard source/*.c) $(wildcard source/**/*.c) lib/PlatinumSrc/audio.c
OBJECTS := $(patsubst source/%.c,bin/%.o,$(SOURCES))
OUT     := arkas

ifeq ($(PLAT),windows)
	CC := x86_64-w64-mingw32-gcc
	override LDLIBS += -lkernel32 -l:libSDL2.a -lole32 -loleaut32 -limm32
	override LDLIBS += -lsetupapi -lversion -lgdi32 -lwinmm -lopengl32
else
	override LDLIBS += -lSDL2 -lGL
endif
LD := $(CC)

override CFLAGS += -std=c99 -Wall -Wextra -Wuninitialized -Wundef -pedantic -Ilib
override LDLIBS += -lm

override CFLAGS += -DAE_BACKEND_GL11 -DAE_AUDIO_PSRC
override CPPFLAGS += -DSDL_MAIN_HANDLED

ifeq ($(BUILD),release)
	override CFLAGS += -O3
	#override CPPFLAGS += -NDEBUG
else
	override CFLAGS += -Og -g
	override LDFLAGS += -Og -g
	ifeq ($(ASAN),y)
		override CFLAGS += -fno-omit-frame-pointer -fsanitize=address
		override LDFLAGS += -fno-omit-frame-pointer -fsanitize=address
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

bin/ui:
	mkdir -p bin/ui

bin/%.o: source/%.c $(call deps,source/%.c) | bin/ bin/backends bin/ui
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

clean:
	rm -r bin

distclean: clean
	rm $(OUT)

.PHONY: all run clean distclean
