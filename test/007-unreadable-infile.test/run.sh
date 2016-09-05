#!/bin/sh

touch foo.in
chmod -r foo.in

${EXECUTABLE} -r 1 -c 1 -i foo.in -s 1 -n -track.raw

if [ $? -eq 0 ]; then
	exit 1
else
	rm -f foo.in
	exit 0
fi
