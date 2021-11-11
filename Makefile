obj = app.obj video.obj gfx.obj ui.obj mouse.obj font8x8.obj
bin = pntless.exe

CC = wcc386
LD = wlink
CFLAGS = -d3 -s -zq -bt=dos

$(bin): $(obj)
	%write objects.lnk $(obj)
	$(LD) debug all name $@ system dos4g file { @objects } $(LDFLAGS)

.c.obj:
	$(CC) -fo=$@ $(CFLAGS) $<

.asm.obj:
	nasm -f obj -o $@ $(ASFLAGS) $<

clean: .symbolic
	del *.obj
	del $(bin)
