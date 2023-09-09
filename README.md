Paintless
=========
A pointless VGA paint program for DOS.

![shots](http://nuclear.mutantstargoat.com/sw/misc/pntless_shots.png)

I didn't mean to write a paint program. I was just hacking aimlessly, one thing
led to another, and a paint program appeared.

License
-------
Copyright (C) 2021-2023 John Tsiombikas <nuclear@mutantstargoat.com>

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
 - 4: filled rectangle tool
 - 5: flood fill tool
 - 6: color picker tool
 - [ / ]: increase/decrease brush size.
 - Tab: toggle UI
 - Backspace: clear canvas
 - U: undo
 - L: load image from file
 - S: save image to file
 - Esc: exit program
