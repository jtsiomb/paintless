#include "ui.h"
#include "gfx.h"

#define TBAR_COLSZ	(TBAR_HEIGHT - 2)

#define UICOL_MAIN	7
#define UICOL_LIT	15
#define UICOL_SHAD	8

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
