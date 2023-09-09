/*
Simple paint program for DOS
Copyright (C) 2021-2023 John Tsiombikas <nuclear@member.fsf.org>

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
#include <stdlib.h>
#include <string.h>
#include "gfx.h"

void draw_brush(unsigned char *fb, int x, int y, int sz, unsigned char col, int op)
{
	int i, j, rad;
	int xor = (op & OP_MASK) == OP_XOR;

	rad = sz >> 1;
	y -= rad;
	x -= rad;
	fb += (y << 8) + (y << 6) + x;

	for(i=0; i<sz; i++) {
		if(y + i < 0) continue;
		if(y + i >= 200) break;
		for(j=0; j<sz; j++) {
			if(x + j < 0) continue;
			if(x + j >= 320) break;

			if(xor) {
				fb[j] ^= col;
			} else {
				fb[j] = col;
			}
		}
		fb += 320;
	}
}

void draw_line(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col, int op)
{
	int i, dx, dy, dx2, dy2, xinc, yinc, err;

	fb += (y0 << 8) + (y0 << 6) + x0;
	dx = x1 - x0;
	dy = y1 - y0;

	if(dx >= 0) {
		xinc = 1;
	} else {
		xinc = -1;
		dx = -dx;
	}
	if(dy >= 0) {
		yinc = 320;
	} else {
		yinc = -320;
		dy = -dy;
	}
	dx2 = dx << 1;
	dy2 = dy << 1;

	if(dx > dy) {
		/* x-major */
		err = dy2 - dx;
		for(i=0; i<=dx; i++) {
			if(op == OP_XOR) {
				*fb ^= col;
			} else {
				*fb = col;
			}
			if(err >= 0) {
				err -= dx2;
				fb += yinc;
			}
			err += dy2;
			fb += xinc;
		}
	} else {
		/* y-major */
		err = dx2 - dy;
		for(i=0; i<=dy; i++) {
			if(op == OP_XOR) {
				*fb ^= col;
			} else {
				*fb = col;
			}
			if(err >= 0) {
				err -= dy2;
				fb += xinc;
			}
			err += dx2;
			fb += yinc;
		}
	}
}

#define SWAP(a, b)	do { int tmp = a; a = b; b = tmp; } while(0)
void draw_rect(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col, int op)
{
	int i, j, dx, dy;
	unsigned char *bptr;

	if(x0 > x1) SWAP(x0, x1);
	if(y0 > y1) SWAP(y0, y1);
	dx = x1 - x0;
	dy = y1 - y0;

	if(!(dx | dy)) return;

	dx++;
	dy++;
	fb += (y0 << 8) + (y0 << 6) + x0;

	if(op & OP_FILL) {
		for(i=0; i<dy; i++) {
			switch(op & OP_MASK) {
			case OP_WR:
				memset(fb, col, dx);
				break;
			case OP_XOR:
				for(j=0; j<dx; j++) {
					fb[j] ^= col;
				}
				break;
			}
			fb += 320;
		}

	} else {

		switch(op & OP_MASK) {
		case OP_WR:
			dy--;
			memset(fb, col, dx);
			memset(fb + (dy << 8) + (dy << 6), col, dx);
			dx--;
			for(i=0; i<dy-1; i++) {
				fb += 320;
				*fb = col;
				fb[dx] = col;
			}
			break;

		case OP_XOR:
			dy--;
			bptr = fb + (dy << 8) + (dy << 6);
			for(i=0; i<dx; i++) {
				fb[i] ^= col;
				*bptr++ ^= col;
			}
			dx--;
			for(i=0; i<dy-1; i++) {
				fb += 320;
				*fb ^= col;
				fb[dx] ^= col;
			}
			break;
		}
	}
}

#define FFSTACK_SIZE	16384
static unsigned long ffstack[FFSTACK_SIZE];
static int fftop;

#define FFPUSH(x, y) \
	do { \
		if(fftop < FFSTACK_SIZE) { \
			ffstack[fftop++] = ((x) & 0xffff) | ((y) << 16); \
		} \
	} while(0)

#define FFPOP(x, y) \
	do { \
		x = ffstack[--fftop] & 0xffff; \
		y = ffstack[fftop] >> 16; \
	} while(0)

#define POFF(x, y)	(((y) << 8) + ((y) << 6) + (x))

void floodfill(unsigned char *fb, int x, int y, unsigned char col)
{
	unsigned char *pptr = fb + POFF(x, y);
	unsigned char prevcol = *pptr;
	if(col == prevcol) return;

	FFPUSH(x, y);

	while(fftop) {
		FFPOP(x, y);
		pptr = fb + POFF(x, y);
		*pptr = col;

		if(x > 0 && pptr[-1] == prevcol) {
			FFPUSH(x - 1, y);
		}
		if(x < 319 && pptr[1] == prevcol) {
			FFPUSH(x + 1, y);
		}
		if(y > 0 && pptr[-320] == prevcol) {
			FFPUSH(x, y - 1);
		}
		if(y < 199 && pptr[320] == prevcol) {
			FFPUSH(x, y + 1);
		}
	}
}
