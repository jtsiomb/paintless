obj = lines.obj gfx.obj ui.obj mouse.obj
bin = lines.exe

CC = wcc386
LD = wlink
CFLAGS = -d3 -s -zq -bt=dos

$(bin): $(obj)
	$(LD) debug all name $@ system dos4g file { $(obj) } $(LDFLAGS)

.c.obj:
	$(CC) -fo=$@ $(CFLAGS) $<

.asm.obj:
	nasm -f obj -o $@ $(ASFLAGS) $<

clean: .symbolic
	del *.obj
	del $(bin)
