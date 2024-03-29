CC = gcc

LIBS := -I./include -lshell32 -lXInput -lgdi32 -lm -ggdb -lShcore -lWinmm -lSDL2
EXT = .exe

ifeq ($(CC),x86_64-w64-mingw32-gcc)
	STATIC = --static
endif

ifneq (,$(filter $(CC),winegcc x86_64-w64-mingw32-gcc))
    detected_OS := Windows
	LIB_EXT = .dll
else
	ifeq '$(findstring ;,$(PATH))' ';'
		detected_OS := Windows
	else
		detected_OS := $(shell uname 2>/dev/null || echo Unknown)
		detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
		detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
		detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
	endif
endif

ifeq ($(detected_OS),Windows)
	LIBS := -I./include -lXInput -ggdb -lWinmm -lshell32 -lgdi32 -lShcore -lm -ldwmapi $(STATIC)
	EXT = .exe
endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := -I./include -lm -framework Foundation -framework AppKit -framework AVFoundation -framework OpenGL -framework CoreVideo -framework CoreMIDI -w $(STATIC)
	EXT = 
endif
ifeq ($(detected_OS),Linux)
    LIBS := -I./include -lXrandr -lX11 -lm $(STATIC)
	EXT = 
endif

all:
	$(CC) rgfw_example.c  -O3 $(LIBS) -I./ -Wall -o rgfw_example$(EXT)

clean:
	rm -f rgfw_example rgfw_example$(EXTT)

debug:
	make clean

	$(CC) rgfw_example.c $(LIBS) -I./ -Wall -D RGFW_DEBUG -o rgfw_example
	cd ../../ && ./examples/RGFW/rgfw_example$(EXT)
