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
#ifndef MOUSE_H_
#define MOUSE_H_

enum {
	MOUSE_LEFT		= 1,
	MOUSE_RIGHT		= 2,
	MOUSE_MIDDLE	= 4
};

#ifdef __cplusplus
extern "C" {
#endif

int have_mouse(void);
void show_mouse(int show);
int read_mouse(int *xp, int *yp);
void set_mouse(int x, int y);
void set_mouse_limits(int xmin, int ymin, int xmax, int ymax);
void set_mouse_xrange(int xmin, int xmax);
void set_mouse_yrange(int ymin, int ymax);
void set_mouse_rate(int xrate, int yrate);

#ifdef __cplusplus
}
#endif

#endif	/* MOUSE_H_ */
