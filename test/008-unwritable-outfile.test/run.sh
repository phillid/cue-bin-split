#!/bin/sh

touch 001-track.raw
chmod -w 001-track.raw

echo -e '0:0:0\n1:0:0' | ${EXECUTABLE} -r 1 -c 1 -i /dev/null -s 1 -n -track.raw

if [ $? -eq 0 ]; then
	exit 1
else
	rm -f 001-track.raw
	exit 0
fi
