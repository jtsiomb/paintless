Simple PC paint program for DOS

License
-------
Copyright (C) 2021 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation.
See COPYING for details.

Build
-----
With a Watcom C environment set up, simply type `wmake` to build.

Usage
-----
A mouse is required. Left mouse button uses the selected tool, right mouse
button cycles between 16 preset colors.

Keyboard controls:
 - 1: brush tool
 - 2: line tool
 - 3: rectangle tool
 - F: toggle between fill/outline for closed shape tools
 - Tab: toggle UI
 - Backspace: clear canvas
 - L: load image from file `lines.ppm`
 - S: save image to file `lines.ppm`
 - Esc: exit program
