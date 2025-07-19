

CC=clang
CFLAGS   = `pkg-config --cflags cairo x11 fontconfig freetype2`
LDFLAGS  = `pkg-config --libs cairo x11 fontconfig freetype2`
TARGET   = mangobar

$(TARGET): main.c font.c block.h command.c
	$(CC) main.c font.c block.c command.c -g -o $(TARGET) $(CFLAGS) $(LDFLAGS)


clean:
	rm -f ./mangobar
