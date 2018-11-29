
Debian
====================
This directory contains files used to package magnachaind/magnachain-qt
for Debian-based Linux systems. If you compile magnachaind/magnachain-qt yourself, there are some useful files here.

## magnachain: URI support ##


magnachain-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install magnachain-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your magnachain-qt binary to `/usr/bin`
and the `../../share/pixmaps/magnachain128.png` to `/usr/share/pixmaps`

magnachain-qt.protocol (KDE)

