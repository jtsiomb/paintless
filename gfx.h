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
