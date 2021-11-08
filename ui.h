#ifndef UI_H_
#define UI_H_

#define TBAR_HEIGHT	12

enum {FRM_IN, FRM_OUT};

void draw_frame(unsigned char *fb, int x0, int y0, int x1, int y1, int style);
void draw_toolbar(void);
void draw_colorbox(unsigned char col);
void draw_cursor(int x, int y, unsigned char col);

#endif	/* UI_H_ */
