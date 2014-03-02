FLAC play 1.2.1-10
=======================

Introduction
------------
	This is a FLAC player plugin for the PM123.

	FLAC (http://flac.sourceforge.net/) is an Open Source lossless audio
	codec developed by Josh Coalson.

Features
--------
	Based on flac-1.2.1 (the latest at the moment)
	Partly support APE and ID3V1 tags (read only)
	Support reply/gain (with APE tags and PM123 v1.32, read only) 

System requirements
-------------------
	OS/2 4.x, ECs (or OS/2 3.x - I guess)
	PM123 1.32 or later (1.31 also appear to work),
		http://glass.os2.spb.ru/software/english/pm123.html
	libc063.dll (latest known at the moment is from 
    	   	ftp://ftp.netlabs.org/pub/libc/libc-0_6_3-csd3.exe)

Warnings and limitations
------------------------
	This version is not hardly tested. Use with care!
	Due to a little sense of VBR in lossless, VBR are not reported at all.

Installation
------------
	- make it sure that libc063.dll located somewhere in your LIBPATH
	- place the file flacplay.dll into the directory where PM123.EXE located
	- start PM123
	- Right-click on the PM123 window to open the "properties" dialog
	- Choose the page "plugins"
	- Press the "add" button in the "decoder plugin" section
	- Choose "flacplay.dll" in the file dialog.
	  Press Ok.
	- Close "PM123 properties" dialog

	Now you can listen "native" FLAC compressed audio files (.flac)
	and Ogg-encapsulated FLAC compressed audio files (.ogg, .oga ?).

De-Installation
---------------
	In case of any trouble with this plugin close PM123 and remove 
	flacplay.dll from the PM123.EXE directory.

License
-------
	This work use libFLAC and libFLAC++,
	* Copyright (C) 2000,2001,2002,2003,2004,2005,2006,2007  Josh Coalson
	see doc/COPYING.Xiph for license terms.

	Based on pm123 plug-ins source, copyrighted by:
	 * Copyright 2004-2007 Dmitry A.Steklenev<glass@ptv.ru>
	 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
	 * Copyright 1997-2003 Taneli Lepp„ <rosmo@sektori.com>
	see doc/COPYRIGHT.html for license terms.

	APE and ID3V1 tags recognition code (id3tag.cpp) is from:
	 * Musepack audio compression
	 * Copyright (C) 1999-2004 Buschmann/Klemm/Piecha/Wolf,
	licensed under the terms of the GNU LGPL 2.1 or later,
	see doc/COPYING.LGPL for full terms.

	Other code, modification and compilation copyrighted by:
	 * Copyright (C) 2008 ntim <ntim@softhome.net>
	and licensed under the GNU GPL version 3 or later,
	see doc/COPYING.GPL for full terms.
	
	The author is not responsible for any damage this program may cause.

Sources
-------
	To meet GNU GPL license terms, all sources and Makefile
	are located in src/ directory.

Contacts
--------
	All questions about this build please send to ntim@softhome.net
	or contact ntim on #os2russian (irc://efnet/os2russian)

2008, ntim
