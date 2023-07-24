# Install
BIN = enterprise

# Flags
CFLAGS += -Wall -Wextra -pedantic -fsanitize=address,leak,undefined -g

SRC = src/main.c
OBJ = $(SRC:.c=.o)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIBS = -lSDL2 -framework OpenGLES -lm
else
	LIBS = -lSDL2 -lGLESv2 -lm
endif

$(BIN): prepare
	$(CC) $(SRC) $(CFLAGS) -o bin/native/$(BIN) $(LIBS)

web: prepare
	emcc $(SRC) -Os -s USE_SDL=2 -o bin/web/index.html --embed-file ProggyClean.ttf

prepare:
	mkdir -p bin/native && mkdir -p bin/web/
