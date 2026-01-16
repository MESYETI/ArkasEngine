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

ifeq ($(STATIC), y)
	override CFLAGS += -static -static-libgcc -static-libstdc++
endif

override CFLAGS += -std=c99 -Wall -Wextra -Wuninitialized -Wundef -pedantic -Ilib -Werror=return-type
override LDLIBS += -lm

override CFLAGS += -DAE_BACKEND_GL11 -DAE_AUDIO_PSRC -DAE_USE_SDL2
override CPPFLAGS += -DSDL_MAIN_HANDLED

ifeq ($(BUILD),release)
	override CFLAGS += -O3
	#override CPPFLAGS += -NDEBUG
else
	override CFLAGS += -O0 -ggdb -fno-omit-frame-pointer
	override LDFLAGS += -O0 -ggdb -fno-omit-frame-pointer
	ifeq ($(ASAN),y)
		override CFLAGS += -fsanitize=address
		override LDFLAGS += -fsanitize=address
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

bin/input:
	mkdir -p bin/input

bin/%.o: source/%.c $(call deps,source/%.c) | bin/ bin/backends bin/ui bin/input
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

clean:
	rm -r bin

distclean: clean
	rm $(OUT)

.PHONY: all run clean distclean
