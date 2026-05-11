CC       ?= gcc
AR       ?= ar

PLAT := $(shell uname -s)

SOURCES := $(shell find source -name '*.c')
SOURCES += lib/PlatinumSrc/audio.c

OBJECTS := $(patsubst %.c,bin/%.o,$(SOURCES))
DEPS    := $(OBJECTS:.o=.d)

CFLAGS += -std=c99
CFLAGS += -Wall -Wextra -Wundef -Wpedantic
CFLAGS += -MMD -MP
CFLAGS += -Ilib

CPPFLAGS += \
    -DAE_BACKEND_GL_LEGACY \
    -DAE_AUDIO_PSRC \
    -DAE_WINDOW_SDL2 \
    -DAE_INPUT_SDL2 \
    -DAE_EVENT_SDL2 \
    -DSDL_MAIN_HANDLED \
    -D_POSIX_C_SOURCE=199309L

LDLIBS += -lm

ifeq ($(BUILD),release)
    CFLAGS += -O3
else
    CFLAGS += -O0 -g3 -fno-omit-frame-pointer
endif

ifeq ($(ASAN),y)
    CFLAGS += -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
endif

ifeq ($(OS),Windows_NT)
    CC := x86_64-w64-mingw32-gcc
    LDLIBS += -lkernel32 -lSDL2 -lopengl32
else
    CFLAGS += -DAE_NET_SOCKET
    LDLIBS += -lSDL2 -lGL
endif

OUT ?= arkas

all: $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

bin/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf bin

distclean: clean
	rm -f $(OUT)

run: $(OUT)
	./$(OUT)

-include $(DEPS)

.PHONY: all clean distclean run
