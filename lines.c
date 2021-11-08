#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "mouse.h"

#define TBAR_HEIGHT	12

enum { OP_WR, OP_XOR };
#define OP_MASK	0x7f
#define OP_FILL	0x80

enum {
	TOOL_BRUSH,
	TOOL_LINE,
	TOOL_RECT,
	NUM_TOOLS
};
#define MAX_TOOL_RAD	32

void draw_toolbar(void);
void draw_colorbox(void);
void draw_brush(unsigned char *fb, int x, int y, int rad, unsigned char col, int op);
void draw_line(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col, int op);
void draw_rect(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col, int op);
void draw_cursor(int x, int y, unsigned char col);
void set_video_mode(int mode);
void wait_vblank(void);
int load_image(const char *fname);
int save_image(const char *fname);

int startx = -1, starty;
unsigned char img[64000];
unsigned char *vmem = (unsigned char*)0xa0000;
unsigned char backbuf[64000 + 320 * 16];
unsigned char *fb = backbuf + 320 * 8;
unsigned char cur_col = 0xf;
int tool, tool_sz = 1;
int show_ui = 1;
int fill;

int main(void)
{
	int c, mx, my, mbn, mbn_prev = 0, mbn_delta;

	set_video_mode(0x13);
	if(!have_mouse()) {
		set_video_mode(3);
		printf("Mouse driver not detected\n");
		return 1;
	}
	set_mouse_xrange(0, 638);
	set_mouse_yrange(0, 199);

	for(;;) {
		if(kbhit()) {
			c = getch();
			switch(c) {
			case 27:
				goto end;

			case '1':
				tool = TOOL_BRUSH;
				break;
			case '2':
				tool = TOOL_LINE;
				break;
			case '3':
				tool = TOOL_RECT;
				break;

			case ']':
				if(tool_sz < MAX_TOOL_RAD) tool_sz++;
				break;
			case '[':
				if(tool_sz > 1) tool_sz--;
				break;

			case 'f':
			case 'F':
				fill ^= OP_FILL;
				break;

			case '\b':
				memset(img, 0, 64000);
				break;

			case '\t':
				show_ui = !show_ui;
				break;

			case 's':
			case 'S':
				save_image("lines.ppm");
				break;

			case 'l':
			case 'L':
				load_image("lines.ppm");
				break;
			}
		}

		mbn = read_mouse(&mx, &my);
		mbn_delta = mbn ^ mbn_prev;
		mbn_prev = mbn;
		mx >>= 1;

		memcpy(fb, img, 64000);

		if(mbn & MOUSE_LEFT) {
			if(startx == -1) {
				startx = mx;
				starty = my;
			}
			switch(tool) {
			case TOOL_BRUSH:
				draw_brush(img, mx, my, tool_sz, cur_col, OP_WR);
				break;
			case TOOL_LINE:
				draw_line(fb, startx, starty, mx, my, 0xf, OP_XOR);
				break;
			case TOOL_RECT:
				draw_rect(fb, startx, starty, mx, my, 0xf, OP_XOR | fill);
				break;
			}
		} else {
			if(startx >= 0) {
				switch(tool) {
				case TOOL_LINE:
					draw_line(img, startx, starty, mx, my, cur_col, OP_WR);
					break;
				case TOOL_RECT:
					draw_rect(img, startx, starty, mx, my, cur_col, OP_WR | fill);
					break;
				}
				startx = starty = -1;
			}
		}

		if(mbn & mbn_delta & MOUSE_RIGHT) {
			cur_col = (cur_col + 1) & 0xf;
		}

		switch(tool) {
		case TOOL_BRUSH:
			draw_brush(fb, mx, my, tool_sz, 0xf, OP_XOR);
			break;

		default:
			draw_cursor(mx, my, 0xf);
		}

		if(show_ui) {
			if(show_ui == 1 && my == 0) {
				show_ui++;
			}
			if(show_ui == 2 && my >= TBAR_HEIGHT) {
				show_ui--;
			}
			if(show_ui >= 2) {
				draw_toolbar();
			}
			draw_colorbox();
		}

		wait_vblank();
		memcpy(vmem, fb, 64000);
	}
end:

	set_video_mode(3);
	return 0;
}

#define UICOL_MAIN	7
#define UICOL_LIT	15
#define UICOL_SHAD	8
enum {FRM_IN, FRM_OUT};
static void draw_frame(unsigned char *fb, int x0, int y0, int x1, int y1, int style)
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

#define TBAR_COLSZ	(TBAR_HEIGHT - 2)
void draw_toolbar(void)
{
	draw_frame(fb, 0, 0, 319, TBAR_HEIGHT-1, FRM_OUT);
}

void draw_colorbox(void)
{
	int x, y;

	x = 319 - TBAR_COLSZ;
	y = 1;
	draw_frame(fb, x, y, x + TBAR_COLSZ - 1, y + TBAR_COLSZ - 1, FRM_IN);
	draw_rect(fb, x + 1, y + 1, x + TBAR_COLSZ - 2, y + TBAR_COLSZ - 2, cur_col, OP_WR | OP_FILL);
}

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
	long x, y, dx, dy, slope, step;

	x = x0 << 8;
	y = y0 << 8;
	dx = (x1 - x0) << 8;
	dy = (y1 - y0) << 8;

	if(!(dx | dy)) return;

	op &= OP_MASK;
	if(op == OP_XOR) {
		fb[(y0 << 8) + (y0 << 6) + x0] ^= col;
	} else {
		fb[(y0 << 8) + (y0 << 6) + x0] = col;
	}

	if(abs(dx) > abs(dy)) {
		/* x-major */
		slope = dx ? (dy << 8) / dx : 0;
		if(dx >= 0) {
			step = 1;
		} else {
			step = -1;
			slope = -slope;
		}

		do {
			x += step << 8;
			y += slope;
			x0 = x >> 8;
			y0 = y >> 8;
			if(op == OP_XOR) {
				fb[(y0 << 8) + (y0 << 6) + x0] ^= col;
			} else {
				fb[(y0 << 8) + (y0 << 6) + x0] = col;
			}
		} while(x >> 8 != x1);
			
	} else {
		/* y-major */
		slope = dy ? (dx << 8) / dy : 0;
		if(dy >= 0) {
			step = 1;
		} else {
			step = -1;
			slope = -slope;
		}

		do {
			x += slope;
			y += step << 8;
			x0 = x >> 8;
			y0 = y >> 8;
			if(op == OP_XOR) {
				fb[(y0 << 8) + (y0 << 6) + x0] ^= col;
			} else {
				fb[(y0 << 8) + (y0 << 6) + x0] = col;
			}
		} while(y >> 8 != y1);
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

void draw_cursor(int x, int y, unsigned char col)
{
	unsigned char *p = fb + (y << 8) + (y << 6) + x;
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

void set_video_mode(int mode)
{
	union REGS regs = {0};
	regs.x.eax = mode & 0xff;
	int386(0x10, &regs, &regs);
}

void wait_vblank(void);
#pragma aux wait_vblank = \
	"mov dx, 0x3da" \
	"l1:" \
	"in al, dx" \
	"and al, 0x8" \
	"jnz l1" \
	"l2:" \
	"in al, dx" \
	"and al, 0x8" \
	"jz l2" \
	modify[al dx];

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
