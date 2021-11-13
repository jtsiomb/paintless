/*
Simple paint program for DOS
Copyright (C) 2021 John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include <direct.h>
#include "app.h"
#include "video.h"
#include "mouse.h"

static unsigned char *vmem = (unsigned char*)0xa0000;
static char cwd[256];
static int quit;

int main(void)
{
	int c, mbn, mbn_prev = 0, mbn_delta;

	getcwd(cwd, sizeof cwd);

	set_video_mode(0x13);
	if(!_dos_getvect(0x33) || !reset_mouse()) {
		set_video_mode(3);
		printf("Mouse driver not detected\n");
		return 1;
	}
	set_mouse_xrange(0, 638);
	set_mouse_yrange(0, 199);

	for(;;) {
		if(kbhit()) {
			c = getch();
			app_keypress(c);
			if(quit) break;
		}

		app_begin_frame();

		mbn = read_mouse(&mx, &my);
		mbn_delta = mbn ^ mbn_prev;
		mbn_prev = mbn;
		mx >>= 1;

		if(mbn_delta & MOUSE_LEFT) {
			app_mouse_button(MOUSE_LEFT, mbn & MOUSE_LEFT ? 1 : 0);
			if(quit) break;
		}
		if(mbn_delta & MOUSE_RIGHT) {
			app_mouse_button(MOUSE_RIGHT, mbn & MOUSE_RIGHT ? 1 : 0);
			if(quit) break;
		}

		app_end_frame();

		wait_vblank();
		memcpy(vmem, framebuf, 64000);
	}

	set_video_mode(3);
	chdir(cwd);
	return 0;
}

void app_quit(void)
{
	quit = 1;
}
