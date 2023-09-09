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
#include <stdint.h>
#include <GL/freeglut.h>
#if defined(unix) || defined(__unix__)
#include <GL/glx.h>
#include <X11/Xlib.h>
#endif
#include "app.h"
#include "mouse.h"

struct event {
	int bn, state;
	int x, y;
	struct event *next;
};
static struct event *evlist, *evtail;

static void display(void);
static void idle(void);
static void reshape(int x, int y);
static void keyb(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

#define WIDTH	320
#define HEIGHT	200
#define ASPECT	1.3333333333333f
static uint32_t pixels[64000];
static int win_width, win_height;
static float win_aspect;
static int tex_xsz = 512;
static int tex_ysz = 256;

static uint32_t palette[256] = {
	0x000000,	/*  0: black */
	0x7f0000,	/*  1: blue */
	0x007f00,	/*  2: green */
	0x7f7f00,	/*  3: cyan */
	0x00007f,	/*  4: red */
	0x7f007f,	/*  5: magenta */
	0x007fc0,	/*  6: brown */
	0xc0c0c0,	/*  7: light grey */
	0x7f7f7f,	/*  8: dark grey */
	0xff0000,	/*  9: light blue */
	0x00ff00,	/* 10: light green */
	0xffff00,	/* 11: light cyan */
	0x0000ff,	/* 12: light red */
	0xff00ff,	/* 13: light magenta */
	0x00ffff,	/* 14: light yellow */
	0xffffff	/* 15: white */
};

#if defined(unix) || defined(__unix__)
static void (*glx_swap_interval_ext)(Display*, Window, int);
static void (*glx_swap_interval_mesa)(int);
static void (*glx_swap_interval_sgi)(int);
#endif


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(640, 480);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("paintless");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_xsz, tex_ysz, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef((float)WIDTH / tex_xsz, (float)HEIGHT / tex_ysz, 1);

#if defined(unix) || defined(__unix__)
	if((glx_swap_interval_ext = (void (*)(Display*, Window, int))glXGetProcAddress("glXSwapIntervalEXT"))) {
		Display *dpy = glXGetCurrentDisplay();
		Window win = glXGetCurrentDrawable();
		glx_swap_interval_ext(dpy, win, 1);

	} else if((glx_swap_interval_mesa = (void (*)(int))glXGetProcAddress("glXSwapIntervalMESA"))) {
		glx_swap_interval_mesa(1);

	} else if((glx_swap_interval_sgi = (void (*)(int))glXGetProcAddress("glXSwapIntervalSGI"))) {
		glx_swap_interval_sgi(1);
	}
#endif
	glutSetCursor(GLUT_CURSOR_NONE);

	glutMainLoop();
	return 0;
}

void app_swap_buffers(void)
{
	int i, idx;
	unsigned char *fbptr = framebuf;
	for(i=0; i<64000; i++) {
		idx = *fbptr++;
		pixels[i] = palette[idx];
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(win_aspect > ASPECT) {
		glScalef(ASPECT / win_aspect, 1, 1);
	} else {
		glScalef(1, win_aspect / ASPECT, 1);
	}

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-1, 1);
	glEnd();

	glutSwapBuffers();
}

void app_quit(void)
{
	exit(0);
}

static void display(void)
{
	int bn;
	struct event *ev;

	glClear(GL_COLOR_BUFFER_BIT);

	app_begin_frame();

	while(evlist) {
		ev = evlist;
		evlist = evlist->next;
		bn = ev->bn;
		app_mouse_button(bn, ev->state);
		free(ev);
	}
	app_end_frame();

	app_swap_buffers();
}

static void idle(void)
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;
}

static void keyb(unsigned char key, int x, int y)
{
	app_keypress(key);
}

static void mouse(int bn, int st, int x, int y)
{
	struct event *ev = calloc(1, sizeof *ev);
	ev->bn = bn == GLUT_LEFT_BUTTON ? MOUSE_LEFT : MOUSE_RIGHT;
	ev->state = st == GLUT_DOWN ? 1 : 0;
	if(evlist) {
		evtail->next = ev;
		evtail = ev;
	} else {
		evlist = evtail = ev;
	}
}

static void motion(int x, int y)
{
	float sx, sy, tx = 0, ty = 0;

	sx = (float)WIDTH / win_width;
	sy = (float)HEIGHT / win_height;
	if(win_aspect > ASPECT) {
		sx *= win_aspect / ASPECT;
		tx = (win_width * ASPECT / win_aspect - win_width) / 2.0f;
	} else {
		sy *= ASPECT / win_aspect;
		ty = (win_height * win_aspect / ASPECT - win_height) / 2.0f;
	}

	mx = (x + tx) * sx;
	my = (y + ty) * sy;

	if(mx < 0) mx = 0;
	if(mx >= WIDTH) mx = WIDTH - 1;
	if(my < 0) my = 0;
	if(my >= HEIGHT) my = HEIGHT - 1;
}
