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
extern int mx, my;

void app_begin_frame(void);
void app_end_frame(void);
void app_keypress(int key);
void app_mouse_button(int bn, int st);
void app_quit(void);

int load_image(const char *fname);
int save_image(const char *fname);

#endif	/* APP_H_ */
