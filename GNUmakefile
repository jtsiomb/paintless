obj = main_unix.o app.o gfx.o ui.o font8x8.o
dep = $(obj:.o=.d)
bin = paintless

CFLAGS = -pedantic -Wall -g	`sdl-config --cflags` -MMD
LDFLAGS = `sdl-config --libs`

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
