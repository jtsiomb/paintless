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
#include <stdarg.h>
#include <conio.h>
#include <direct.h>
#include "app.h"
#include "video.h"
#include "ui.h"
#include "gfx.h"
#include "mouse.h"

enum {
	ICON_DIR = 1,
	ICON_NEW,
	ICON_OPEN,
	ICON_SAVE,
	ICON_BRUSH,
	ICON_LINE,
	ICON_RECT,
	ICON_FILLRECT,
	ICON_FLOOD,
	ICON_PICK
};

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
	int i, x = 2;
	draw_frame(framebuf, 0, 0, 319, TBAR_HEIGHT-1, FRM_OUT);

	for(i=0; i<3; i++) {
		draw_frame(framebuf, x, 1, x + 11, 10, FRM_OUT);
		draw_glyph(framebuf, x + 2, 2, ICON_NEW + i, 0, UICOL_MAIN);
		x += 13;
	}
	x += 5;
	for(i=0; i<NUM_TOOLS; i++) {
		draw_frame(framebuf, x, 1, x + 11, 10, tool == i ? FRM_IN : FRM_OUT);
		draw_glyph(framebuf, x + 2, 2, ICON_BRUSH + i, 0, UICOL_MAIN);
		x += 13;
	}

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
	case 5: p[-5] ^= col;
	case 4: p[-4] ^= col;
	case 3: p[-3] ^= col;
	case 2: p[-2] ^= col;
	case 0: break;
	}
	switch(x) {
	default:
	case 314: p[5] ^= col;
	case 315: p[4] ^= col;
	case 316: p[3] ^= col;
	case 317: p[2] ^= col;
	case 319: break;
	}

	p[1600] ^= col; p[640] ^= col; p[960] ^= col; p[1280] ^= col;
	p[-1600] ^= col; p[-640] ^= col; p[-960] ^= col; p[-1280] ^= col;
}

static int cur_x, cur_y;
static unsigned char fgcol = 0xf, bgcol = 0;
extern unsigned char font_8x8[];

void draw_glyph(unsigned char *fb, int x, int y, char c, unsigned char col, unsigned char bgcol)
{
	int i, j;
	unsigned char *gptr, g;

	if(c < 1 || c > 127) return;

	fb += (y << 8) + (y << 6) + x;
	gptr = font_8x8 + (c << 3);
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

static int substr_len(const char *str)
{
	const char *s = str;
	while(*s && *s != '*' && *s != '?') s++;
	return s - str;
}

static int match_suffix(const char *name, const char *suffix)
{
	int len, slen;

	if(!suffix || !*suffix) return 1;
	while(*suffix == '*') suffix++;

	len = strlen(name);
	slen = strlen(suffix);
	if(slen > len) return 0;

	name += len - slen;
	while(*name) {
		if(tolower(*name++) != tolower(*suffix++)) return 0;
	}
	return 1;
}

struct dir_entry {
	char name[16];
	int dir;
};

static struct dir_entry *load_dir(const char *dirname, const char *filter, int *numret)
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
		if(dent->d_attr == _A_SUBDIR || match_suffix(dent->d_name, filter)) {
			strcpy(entries[num].name, dent->d_name);
			entries[num].dir = dent->d_attr == _A_SUBDIR;
			num++;
		}
	}
	closedir(dir);

	*numret = num;
	return entries;
}

#define FONT_SZ		8
#define ICON_SZ		8
#define FRM_X		2
#define FRM_Y		12
#define FRM_IN_X	(FRM_X + 1)
#define FRM_IN_Y	(FRM_Y + 1)
#define NUMLINES	22
#define FRM_IN_XSZ	(ICON_SZ + 12 * FONT_SZ)
#define FRM_IN_YSZ	(NUMLINES * FONT_SZ)
#define FRM_XSZ		(FRM_IN_XSZ + 2)
#define FRM_YSZ		(FRM_IN_YSZ + 2)
#define FRM2_X		(FRM_X * 2 + FRM_XSZ + 1)
#define FRM2_IN_X	(FRM2_X + 1)
#define TX_X		(FRM2_X + FRM_XSZ + 2)
#define TX_Y		(FRM_Y + 10)

int file_dialog(int type, const char *dirname, const char *filter, char *pathbuf, int bufsz)
{
	static const int coloffs[] = {FRM_IN_X, FRM2_IN_X};
	int mx, my, mbn, mbnprev, mbndelta;
	int i, c, num_entries, column, ypos, textcol;
	struct dir_entry *entries;
	int extkey = 0, cursel = 0;
	char txfield[16] = {0};
	int txcur = 0;


	if(!(entries = load_dir(dirname, filter, &num_entries))) {
		return -1;
	}

	for(;;) {
		memset(framebuf, UICOL_MAIN, 64000);
		gmoveto(1, 1);
		gcolor(0, UICOL_MAIN);

		draw_frame(framebuf, FRM_X, FRM_Y, FRM_X + FRM_XSZ, FRM_Y + FRM_YSZ, FRM_IN);
		draw_frame(framebuf, FRM2_X, FRM_Y, FRM2_X + FRM_XSZ, FRM_Y + FRM_YSZ, FRM_IN);
		gprintf(framebuf, type == FDLG_SAVE ? "Save file ..." : "Open file ...");

		column = -1;
		for(i=0; i<num_entries; i++) {
			if(i % NUMLINES == 0) {
				column++;
				ypos = FRM_IN_Y;
				if(column > 1) break;
			}
			gmoveto(coloffs[column], ypos);
			ypos += FONT_SZ;
			if(cursel == i) {
				gcolor(0, 3);
			} else {
				gcolor(0, UICOL_MAIN);
			}
			if(entries[i].dir) {
				gprintf(framebuf, "%c%-12s", ICON_DIR, entries[i].name);
			} else {
				gprintf(framebuf, " %-12s", entries[i].name);
			}
		}

		draw_text(framebuf, TX_X, TX_Y - 10, "Filename:", 0, UICOL_MAIN);
		draw_frame(framebuf, TX_X, TX_Y, TX_X + 2 + 8 * 12, TX_Y + 9, FRM_IN);
		if(!entries[cursel].dir || txcur) {
			gcolor(0, UICOL_MAIN);
			gmoveto(TX_X + 1, TX_Y + 1);
			gprintf(framebuf, "%-12s", txcur ? txfield : entries[cursel].name);
		}

		mbn = read_mouse(&mx, &my);
		mx >>= 1;
		mbndelta = mbn ^ mbnprev;
		mbnprev = mbn;

		draw_cursor(mx, my, 0xff);

		wait_vblank();
		memcpy((void*)0xa0000, framebuf, 64000);

		if(kbhit()) {
			c = getch();
			if(!extkey) {
				switch(c) {
				case 27:
					goto end;

				case '\n':
				case '\r':
					if(entries[cursel].dir) {
						if(chdir(entries[cursel].name) == 0) {
							free(entries);
							if(!(entries = load_dir(".", filter, &num_entries))) {
								return -1;
							}
							cursel = 0;
						}
					} else {
						strcpy(pathbuf, entries[cursel].name);
						free(entries);
						return 0;
					}
					break;

				case 0:
					extkey = 1;
					break;

				case '\b':
					if(txcur) {
						txfield[--txcur] = 0;
					}
					break;

				default:
					if(type == FDLG_SAVE && (isalnum(c) || c == '.') && txcur < 12) {
						txfield[txcur++] = c;
						txfield[txcur] = 0;
					}
				}
			} else {
				extkey = 0;
				switch(c) {
				case 'H':	/* up arrow */
					if(cursel > 0) cursel--;
					break;
				case 'K':	/* left arrow */
					cursel -= NUMLINES;
					if(cursel < 0) cursel = 0;
					break;
				case 'P':	/* down arrow */
					if(cursel < num_entries - 1) {
						cursel++;
					}
					break;
				case 'M':	/* right arrow */
					cursel += NUMLINES;
					if(cursel >= num_entries) {
						cursel = num_entries - 1;
					}
					break;
				}
			}
		}
	}

end:
	free(entries);
	return -1;
}
