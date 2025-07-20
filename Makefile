

CC=clang
CFLAGS   = `pkg-config --cflags cairo x11 fontconfig freetype2 inih`
LDFLAGS  = `pkg-config --libs cairo x11 fontconfig freetype2 inih`
TARGET   = mangobar
PREFIX   = /usr/local
BINDIR   = $(PREFIX)/bin


$(TARGET): main.c font.c block.h command.c x11utils.c config.c
	$(CC) main.c font.c block.c command.c x11utils.c config.c -g -o $(TARGET) $(CFLAGS) $(LDFLAGS)


install: $(TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)



clean:
	rm -f ./mangobar
