

CC=clang
CFLAGS   = `pkg-config --cflags cairo x11 fontconfig freetype2`
LDFLAGS  = `pkg-config --libs cairo x11 fontconfig freetype2`
TARGET   = mangobar
PREFIX   = /usr/local
BINDIR   = $(PREFIX)/bin


$(TARGET): main.c font.c block.h command.c
	$(CC) main.c font.c block.c command.c -g -o $(TARGET) $(CFLAGS) $(LDFLAGS)


install: $(TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)



clean:
	rm -f ./mangobar
