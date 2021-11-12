#ifndef APP_H_
#define APP_H_

enum {
	TOOL_BRUSH,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_FILLRECT,
	TOOL_FLOOD,
	TOOL_PICK,
	NUM_TOOLS
};

extern unsigned char *framebuf;
extern unsigned char *img;
extern int tool;

int load_image(const char *fname);
int save_image(const char *fname);

#endif	/* APP_H_ */
