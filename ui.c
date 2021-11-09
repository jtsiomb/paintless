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
#include <stdarg.h>
#include <conio.h>
#include <direct.h>
#include "video.h"
#include "ui.h"
#include "gfx.h"

#define TBAR_COLSZ	(TBAR_HEIGHT - 2)

#define UICOL_MAIN	7
#define UICOL_LIT	15
#define UICOL_SHAD	8

extern int mx, my;
extern unsigned char *framebuf;

void draw_frame(unsigned char *fb, int x0, int y0, int x1, int y1, int style)
{
	int i, dx, dy, tcol, bcol;

	if(style == FRM_OUT) {
		tcol = UICOL_LIT;
		bcol = UICOL_SHAD;
	} else {
		tcol = UICOL_SHAD;
		bcol = UICOL_LIT;
	}

	dx = x1 - x0;
	dy = y1 - y0;

	draw_rect(fb, x0 + 1, y0 + 1, x1 - 1, y1 - 1, UICOL_MAIN, OP_WR | OP_FILL);
	fb += (y0 << 8) + (y0 << 6) + x0;
	memset(fb, tcol, dx + 1);
	memset(fb + (dy << 8) + (dy << 6), bcol, dx + 1);
	for(i=0; i<dy; i++) {
		fb[dx] = bcol;
		fb += 320;
		*fb = tcol;
	}
}

void draw_toolbar(void)
{
	draw_frame(framebuf, 0, 0, 319, TBAR_HEIGHT-1, FRM_OUT);
	gmoveto(319 - TBAR_COLSZ - 64, 2);
	gcolor(0, UICOL_MAIN);
	gprintf(framebuf, "%3d,%d", mx, my);
}

void draw_colorbox(unsigned char col)
{
	int x, y;

	x = 319 - TBAR_COLSZ;
	y = 1;
	draw_frame(framebuf, x, y, x + TBAR_COLSZ - 1, y + TBAR_COLSZ - 1, FRM_IN);
	draw_rect(framebuf, x + 1, y + 1, x + TBAR_COLSZ - 2, y + TBAR_COLSZ - 2, col, OP_WR | OP_FILL);
}

void draw_cursor(int x, int y, unsigned char col)
{
	unsigned char *p = framebuf + (y << 8) + (y << 6) + x;
	switch(x) {
	default:
	case 4: p[-4] ^= col;
	case 3: p[-3] ^= col;
	case 2: p[-2] ^= col;
	case 1: p[-1] ^= col;
	case 0: break;
	}
	switch(x) {
	default:
	case 315: p[4] ^= col;
	case 316: p[3] ^= col;
	case 317: p[2] ^= col;
	case 318: p[1] ^= col;
	case 319: break;
	}

	p[320] ^= col; p[640] ^= col; p[960] ^= col; p[1280] ^= col;
	p[-320] ^= col; p[-640] ^= col; p[-960] ^= col; p[-1280] ^= col;
}

static int cur_x, cur_y;
static unsigned char fgcol = 0xf, bgcol = 0;
extern unsigned char font_8x8[];

void draw_glyph(unsigned char *fb, int x, int y, char c, unsigned char col, unsigned char bgcol)
{
	int i, j;
	unsigned char *gptr, g;

	if(c < 32 || c > 127) return;

	fb += (y << 8) + (y << 6) + x;
	gptr = font_8x8 + ((c - 32) << 3);
	for(i=0; i<8; i++) {
		g = *gptr++;
		for(j=0; j<8; j++) {
			fb[j] = g & 0x80 ? col : bgcol;
			g <<= 1;
		}
		fb += 320;
	}
}

void draw_text(unsigned char *fb, int x, int y, const char *str, unsigned char col, unsigned char bgcol)
{
	while(*str) {
		draw_glyph(fb, x, y, *str++, col, bgcol);
		x += 8;
		if(x >= 320 - 8) return;
	}
}

void gcolor(unsigned char fg, unsigned char bg)
{
	fgcol = fg;
	bgcol = bg;
}

void gmoveto(int x, int y)
{
	cur_x = x;
	cur_y = y;
}

void gprintf(unsigned char *fb, const char *fmt, ...)
{
	char buf[256];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	draw_text(fb, cur_x, cur_y, buf, fgcol, bgcol);
}

struct dir_entry {
	char name[16];
	int dir;
};

static struct dir_entry *load_dir(const char *dirname, int *numret)
{
	int i, num = 0;
	DIR *dir;
	struct dirent *dent;
	struct dir_entry *entries;

	if(!(dir = opendir(dirname ? dirname : "."))) {
		return 0;
	}
	while((dent = readdir(dir))) {
		if(strcmp(dent->d_name, ".") == 0) continue;
		num++;
	}
	rewinddir(dir);

	if(!(entries = malloc(num * sizeof *entries))) {
		return 0;
	}
	num = 0;

	while((dent = readdir(dir))) {
		if(strcmp(dent->d_name, ".") == 0) continue;
		strcpy(entries[num].name, dent->d_name);
		entries[num].dir = dent->d_attr == _A_SUBDIR;
		num++;
	}
	closedir(dir);

	*numret = num;
	return entries;
}

static unsigned char tcurcol = 7;
static int tx, ty;

static void tcolor(unsigned char fg, unsigned char bg)
{
	tcurcol = (bg << 4) | (fg & 0xf);
}

static void tmoveto(int x, int y)
{
	tx = x;
	ty = y;
}

static void tputchar(int c)
{
	unsigned char *dest = (unsigned char*)0xb8000 + ty * 160 + (tx << 1);
	*dest++ = c;
	*dest++ = tcurcol;
	if(++tx >= 80) {
		tx = 0;
		if(++ty >= 25) ty = 0;
	}
}

static void tprintf(const char *fmt, ...)
{
	char buf[256];
	char *s = buf;
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);

	while(*s) tputchar(*s++);
}

int file_dialog(int type, const char *dirname, const char *filter, char *pathbuf, int bufsz)
{
	int i, c, num_entries;
	struct dir_entry *entries;
	int extkey = 0, cursel = 0;

	set_video_mode(3);

	if(!(entries = load_dir(dirname, &num_entries))) {
		return -1;
	}

	for(;;) {
		memset(0xb8000, 0, 80 * 25 * 2);
		tmoveto(0, 0);
		tcolor(4, 0);
		tprintf("File selection dialog");

		for(i=0; i<num_entries; i++) {
			if(i > 24) break;
			tmoveto(2, i + 1);
			if(cursel == i) {
				tcolor(0, 3);
			} else {
				tcolor(7, 0);
			}
			if(entries[i].dir) {
				if(cursel != i) {
					tcolor(0xf, 0);
				}
				tprintf("[%s]", entries[i].name);
			} else {
				tprintf(" %s", entries[i].name);
			}
		}

		c = getch();
		if(!extkey) {
			switch(c) {
			case 27:
				goto end;

			case '\n':
				break;

			case 0:
				extkey = 1;
				break;
			}
		} else {
			extkey = 0;
			switch(c) {
			case 'H':	/* up arrow */
				if(cursel > 0) cursel--;
				break;
			case 'K':	/* left arrow */
				cursel = 0;
				break;
			case 'P':	/* down arrow */
				if(cursel < num_entries - 1) {
					cursel++;
				}
				break;
			case 'M':	/* right arrow */
				cursel = num_entries - 1;
				break;
			}
		}
	}

end:
	free(entries);
	set_video_mode(0x13);
	return -1;
}
