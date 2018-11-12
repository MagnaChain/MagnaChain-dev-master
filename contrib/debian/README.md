
Debian
====================
This directory contains files used to package bitcoind/celllink-qt
for Debian-based Linux systems. If you compile bitcoind/celllink-qt yourself, there are some useful files here.

## celllink: URI support ##


celllink-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install celllink-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your celllink-qt binary to `/usr/bin`
and the `../../share/pixmaps/bitcoin128.png` to `/usr/share/pixmaps`

celllink-qt.protocol (KDE)

