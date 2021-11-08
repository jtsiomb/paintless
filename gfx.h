#ifndef GFX_H_
#define GFX_H_

enum { OP_WR, OP_XOR };
#define OP_MASK	0x7f
#define OP_FILL	0x80

void draw_brush(unsigned char *fb, int x, int y, int rad, unsigned char col, int op);
void draw_line(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col, int op);
void draw_rect(unsigned char *fb, int x0, int y0, int x1, int y1, unsigned char col, int op);
void floodfill(unsigned char *fb, int x, int y, unsigned char col);

#endif	/* GFX_H_ */
