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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "app.h"
#include "mouse.h"
#include "gfx.h"
#include "ui.h"

#define MAX_TOOL_RAD	32

int mx, my;
int startx = -1, starty;
unsigned char imgbuf[2][64000];
unsigned char *img = imgbuf[0];
unsigned char *undobuf = imgbuf[1];
int curimg;
unsigned char backbuf[64000 + 320 * 16];
unsigned char *framebuf = backbuf + 320 * 8;
unsigned char cur_col = 0xf;
int tool, tool_sz = 1;
int show_ui = 2;
int fill;

void save_undo(void);



void app_begin_frame(void)
{
	memcpy(framebuf, img, 64000);
}

void app_end_frame(void)
{
	if(startx != -1) {	/* dragging */
		switch(tool) {
		case TOOL_BRUSH:
			draw_brush(img, mx, my, tool_sz, cur_col, OP_WR);
			break;
		case TOOL_LINE:
			draw_line(framebuf, startx, starty, mx, my, 0xf, OP_XOR);
			break;
		case TOOL_RECT:
			draw_rect(framebuf, startx, starty, mx, my, 0xf, OP_XOR);
			break;
		case TOOL_FILLRECT:
			draw_rect(framebuf, startx, starty, mx, my, 0xf, OP_XOR | OP_FILL);
			break;
		}
	}

	if(show_ui) {
		if(show_ui >= 2) {
			draw_toolbar();
		}
		draw_colorbox(cur_col);
	}

	if(tool == TOOL_BRUSH && (show_ui < 2 || my > TBAR_HEIGHT)) {
		draw_brush(framebuf, mx, my, tool_sz, cur_col, OP_WR);
	} else {
		draw_brush(framebuf, mx, my, 1, 0xf, OP_XOR);
	}
	draw_cursor(mx, my, 0xf);
}

void app_keypress(int key)
{
	char path[256];

	switch(key) {
	case 27:
		app_quit();
		break;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		tool = key - '1';
		break;

	case ']':
		if(tool_sz < MAX_TOOL_RAD) tool_sz++;
		break;
	case '[':
		if(tool_sz > 1) tool_sz--;
		break;

	case '\b':
		save_undo();
		memset(img, 0, 64000);
		break;

	case '\t':
		show_ui = (show_ui + 1) % 3;
		break;

	case 'u':
	case 'U':
		undobuf = imgbuf[curimg & 1];
		img = imgbuf[++curimg & 1];
		break;

	case 's':
	case 'S':
		if(file_dialog(FDLG_SAVE, 0, "*.ppm", path, sizeof path) != -1) {
			save_image(path);
		}
		break;

	case 'l':
	case 'L':
		if(file_dialog(FDLG_OPEN, 0, "*.ppm", path, sizeof path) != -1) {
			load_image(path);
		}
		break;
	}
}

void app_mouse_button(int bn, int st)
{
	if(show_ui > 1 && my < TBAR_HEIGHT) {
		if(bn == MOUSE_LEFT) {
			uibutton(st ? MOUSE_LEFT : 0, mx, my);
		}
		return;
	}

	if(st) {
		if(bn == MOUSE_LEFT) {
			if(startx == -1) {
				startx = mx;
				starty = my;
			}

			switch(tool) {
			case TOOL_BRUSH:
				save_undo();
				break;
			case TOOL_FLOOD:
				save_undo();
				floodfill(img, mx, my, cur_col);
				break;
			case TOOL_PICK:
				cur_col = img[(my << 8) + (my << 6) + mx];
				break;
			}

		} else if(bn == MOUSE_RIGHT) {
			cur_col = (cur_col + 1) & 0xf;
		}

	} else {	/* release */
		if(startx >= 0) {
			switch(tool) {
			case TOOL_LINE:
				save_undo();
				draw_line(img, startx, starty, mx, my, cur_col, OP_WR);
				break;
			case TOOL_RECT:
				save_undo();
				draw_rect(img, startx, starty, mx, my, cur_col, OP_WR);
				break;
			case TOOL_FILLRECT:
				save_undo();
				draw_rect(img, startx, starty, mx, my, cur_col, OP_WR | OP_FILL);
				break;
			}
			startx = starty = -1;
		}
	}
}


static unsigned char colors[][3] = {
	{0, 0, 0},				/*  0: black */
	{0, 0, 0x7f},			/*  1: blue */
	{0, 0x7f, 0},			/*  2: green */
	{0, 0x7f, 0x7f},		/*  3: cyan */
	{0x7f, 0, 0},			/*  4: red */
	{0x7f, 0, 0x7f},		/*  5: magenta */
	{0xc0, 0x7f, 0},		/*  6: brown */
	{0xc0, 0xc0, 0xc0},		/*  7: light grey */
	{0x7f, 0x7f, 0x7f},		/*  8: dark grey */
	{0, 0, 0xff},			/*  9: light blue */
	{0, 0xff, 0},			/* 10: light green */
	{0, 0xff, 0xff},		/* 11: light cyan */
	{0xff, 0, 0},			/* 12: light red */
	{0xff, 0, 0xff},		/* 13: light magenta */
	{0xff, 0xff, 0},		/* 14: light yellow */
	{0xff, 0xff, 0xff}		/* 15: white */
};

int load_image(const char *fname)
{
	int i, j, r, g, b, cidx, res = -1;
	FILE *fp;
	char buf[256];
	char *line;
	unsigned char *ptr = img;
	int width, height;
	int state = 0;

	if(!(fp = fopen(fname, "rb"))) {
		fprintf(stderr, "failed to open file: %s\n", fname);
		return -1;
	}

	if(!fgets(buf, sizeof buf, fp) || memcmp(buf, "P6", 2) != 0) {
		fprintf(stderr, "invalid image file: %s\n", fname);
		goto err;
	}
	while(fgets(buf, sizeof buf, fp)) {
		line = buf;
		while(*line && isspace(*line)) line++;
		if(*line == '#') continue;	/* skip comments */
		if(state == 0) {
			/* expecting dimensions */
			if(sscanf(line, "%d %d", &width, &height) != 2) {
				fprintf(stderr, "invalid image file: %s\n", fname);
				goto err;
			}
			if(width != 320 || height != 200) {
				fprintf(stderr, "invalid image size: %dx%d\n", width, height);
				goto err;
			}
			state++;
		} else {
			if(atoi(line) != 255) {
				fprintf(stderr, "invalid file or color depth: %d\n", atoi(line));
				goto err;
			}
			break;
		}
	}

	for(i=0; i<64000; i++) {
		r = fgetc(fp);
		g = fgetc(fp);
		b = fgetc(fp);
		if(feof(fp)) {
			fprintf(stderr, "unexpected EOF while reading: %s\n", fname);
			goto err;
		}
		cidx = 0;
		for(j=0; j<16; j++) {
			if(r == colors[j][0] && g == colors[j][1] && b == colors[j][2]) {
				cidx = j;
				break;
			}
		}
		*ptr++ = cidx;
	}

	res = 0;
err:
	fclose(fp);
	return res;
}

int save_image(const char *fname)
{
	int i, c;
	FILE *fp;
	unsigned char *ptr = img;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "failed to write file: %s\n", fname);
		return -1;
	}
	fprintf(fp, "P6\n320 200\n255\n");
	for(i=0; i<64000; i++) {
		c = *ptr++;
		fputc(colors[c][0], fp);
		fputc(colors[c][1], fp);
		fputc(colors[c][2], fp);
	}
	fclose(fp);
	return 0;
}

void save_undo(void)
{
	memcpy(undobuf, img, 64000);
}
