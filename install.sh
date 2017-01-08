#! /bin/sh

SS=nee-tante-julia
CF=/etc/X11/app-defaults/XScreenSaver

grep -q $SS $CF

if [ $? -ne 0 ]
then
	sed -i 's/^\*programs:/*programs: \\\n	nee-tante-julia		\\n/g' $CF
fi

CF2=~/.xscreensaver

grep -q $SS $CF2

if [ $? -ne 0 ]
then
	sed -i 's/^\*programs:/*programs:\n	nee-tante-julia		\\n\\/g' $CF2
fi
