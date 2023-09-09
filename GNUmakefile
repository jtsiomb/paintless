src = main_gl.c app.c font8x8.c gfx.c ui.c
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = paintless

CFLAGS = -pedantic -Wno-pointer-sign -Wall -g -MMD
LDFLAGS = -lGL -lglut

sys := $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(src:.c=.wo)
	dep = $(src:.c=.wd)
	bin = paintless-win32.exe
	LDFLAGS = -static-libgcc -lmingw32 -mconsole -lopengl32 -lfreeglut
endif


$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.SUFFIXES: .wo

.c.wo:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: crosswin
crosswin:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw

.PHONY: crosswin-clean
crosswin-clean:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw clean

.PHONY: crosswin-cleandep
crosswin-cleandep:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw cleandep
