#ifndef UI_H_
#define UI_H_

#define TBAR_HEIGHT	12

enum {FRM_IN, FRM_OUT};

void draw_frame(unsigned char *fb, int x0, int y0, int x1, int y1, int style);
void draw_toolbar(void);
void draw_colorbox(unsigned char col);
void draw_cursor(int x, int y, unsigned char col);
void draw_glyph(unsigned char *fb, int x, int y, char c, unsigned char col,
		unsigned char bgcol);
void draw_text(unsigned char *fb, int x, int y, const char *str, unsigned char col,
		unsigned char bgcol);

void gcolor(unsigned char fg, unsigned char bg);
void gmoveto(int x, int y);
void gprintf(unsigned char *fb, const char *fmt, ...);

#endif	/* UI_H_ */
