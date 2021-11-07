#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <dos.h>
#include "mouse.h"

void draw_line(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col);
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

int main(void)
{
	int mx, my, mbn, mbn_prev = 0, mbn_delta;

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
			switch(getch()) {
			case 27:
				goto end;

			case '\b':
				memset(img, 0, 64000);
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
			draw_line(fb, startx, starty, mx, my, cur_col);
		} else {
			if(startx >= 0) {
				draw_line(img, startx, starty, mx, my, cur_col);
				startx = starty = -1;
			}
		}

		if(mbn & mbn_delta & MOUSE_RIGHT) {
			cur_col = (cur_col + 1) & 0xf;
			if(!cur_col) cur_col++;
		}
	
		draw_cursor(mx, my, cur_col);

		wait_vblank();
		memcpy(vmem, fb, 64000);
	}
end:

	set_video_mode(3);
	return 0;
}

void draw_line(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col)
{
	long x, y, dx, dy, slope, step;

	x = x0 << 8;
	y = y0 << 8;
	dx = (x1 - x0) << 8;
	dy = (y1 - y0) << 8;

	if(abs(dx) > abs(dy)) {
		/* x-major */
		slope = dx ? (dy << 8) / dx : 0;
		if(dx >= 0) {
			step = 1;
		} else {
			step = -1;
			slope = -slope;
		}

		while(x >> 8 != x1) {
			x0 = x >> 8;
			y0 = y >> 8;
			fb[(y0 << 8) + (y0 << 6) + x0] = col;
			x += step << 8;
			y += slope;
		}
			
	} else {
		/* y-major */
		slope = dy ? (dx << 8) / dy : 0;
		if(dy >= 0) {
			step = 1;
		} else {
			step = -1;
			slope = -slope;
		}

		while(y >> 8 != y1) {
			x0 = x >> 8;
			y0 = y >> 8;
			fb[(y0 << 8) + (y0 << 6) + x0] = col;
			x += slope;
			y += step << 8;
		}
	}
}

void draw_cursor(int x, int y, unsigned char col)
{
	unsigned char *p = fb + (y << 8) + (y << 6) + x;
	switch(x) {
	default:
	case 4: p[-4] = col;
	case 3: p[-3] = col;
	case 2: p[-2] = col;
	case 1: p[-1] = col;
	case 0: break;
	}
	switch(x) {
	default:
	case 315: p[4] = col;
	case 316: p[3] = col;
	case 317: p[2] = col;
	case 318: p[1] = col;
	case 319: break;
	}
	p[320] = p[640] = p[960] = p[1280] = col;
	p[-320] = p[-640] = p[-960] = p[-1280] = col;
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
