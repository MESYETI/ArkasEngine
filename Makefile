SOURCES := $(wildcard source/*.c) $(wildcard source/**/*.c) lib/PlatinumSrc/audio.c
OBJECTS := $(patsubst lib/PlatinumSrc/%.c,bin/PlatinumSrc_%.o,$(patsubst source/%.c,bin/%.o,$(SOURCES)))
PLAT    := $(shell uname -s)

ifeq ($(PLAT),NetBSD)
	override CPPFLAGS += -I/usr/X11R7/include -I/usr/pkg/include
	override LDFLAGS += -L/usr/X11R7/lib -L/usr/pkg/lib -Wl,-R/usr/X11R7/lib -Wl,-R/usr/pkg/lib
endif

ifeq ($(PLAT),windows)
	CC := x86_64-w64-mingw32-gcc
	override LDLIBS += -lkernel32 -l:libSDL2.a -lole32 -loleaut32 -limm32
	override LDLIBS += -lsetupapi -lversion -lgdi32 -lwinmm -lopengl32
else
	override CFLAGS += -DAE_NET_SOCKET
	override LDLIBS += -lSDL2 -lGL
endif
LD := $(CC)

ifeq ($(LIB),y)
	OUT := libArkasEngine.a
else
	OUT := arkas
	override CFLAGS += -DAE_STANDALONE
endif

ifeq ($(STATIC), y)
	override CFLAGS += -static -static-libgcc -static-libstdc++
endif

override CFLAGS += -std=c99 -Wall -Wextra -Wuninitialized -Wundef -pedantic -Ilib -Werror=return-type
override LDLIBS += -lm

override CPPFLAGS += -DAE_BACKEND_GL_LEGACY -DAE_AUDIO_PSRC
override CPPFLAGS += -DAE_WINDOW_SDL2 -DAE_INPUT_SDL2 -DAE_EVENT_SDL2
override CPPFLAGS += -DSDL_MAIN_HANDLED -D_POSIX_C_SOURCE=199309L

ifeq ($(BUILD),release)
	override CFLAGS += -O3
	#override CPPFLAGS += -NDEBUG
else
	override CFLAGS += -O0 -ggdb -fno-omit-frame-pointer
	override LDFLAGS += -O0 -ggdb -fno-omit-frame-pointer
	ifeq ($(ASAN),y)
		override CFLAGS += -fsanitize=address,undefined
		override LDFLAGS += -fsanitize=address,undefined
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

arkas: $(OBJECTS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

libArkasEngine.a: $(OBJECTS)
	ar rcs libArkasEngine.a $(OBJECTS)

bin/:
	mkdir -p bin

bin/backends:
	mkdir -p bin/backends

bin/ui:
	mkdir -p bin/ui

bin/input:
	mkdir -p bin/input

bin/window:
	mkdir -p bin/window

bin/event:
	mkdir -p bin/event

bin/platform:
	mkdir -p bin/platform

bin/audio:
	mkdir -p bin/audio

bin/mapEditor:
	mkdir -p bin/mapEditor

bin/PlatinumSrc_%.o: lib/PlatinumSrc/%.c $(call deps,lib/PlatinumSrc/%.c) | bin/ bin/backends bin/ui bin/input bin/window bin/event bin/platform bin/audio bin/mapEditor
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

bin/%.o: source/%.c $(call deps,source/%.c) | bin/ bin/backends bin/ui bin/input bin/window bin/event bin/platform bin/audio bin/mapEditor
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@

clean:
	-rm -r bin

distclean: clean
	rm $(OUT)

.PHONY: all run clean distclean install
