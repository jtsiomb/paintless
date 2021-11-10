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
#ifndef UI_H_
#define UI_H_

#define TBAR_HEIGHT	12

enum {FRM_IN, FRM_OUT};
enum {FDLG_OPEN, FDLG_SAVE};

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

void uibutton(int st, int x, int y);

int file_dialog(int type, const char *dirname, const char *filter, char *pathbuf, int bufsz);

#endif	/* UI_H_ */
