obj = main_gl.o app.o gfx.o ui.o font8x8.o
dep = $(obj:.o=.d)
bin = paintless

CFLAGS = -pedantic -Wno-pointer-sign -Wall -g -MMD
LDFLAGS = -lGL -lglut

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
