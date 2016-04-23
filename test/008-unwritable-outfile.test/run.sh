#!/bin/sh

touch track.raw
chmod -w track.raw

echo -e '0:0:0\n1:0:0' | ${EXECUTABLE} -r 1 -c 1 -i /dev/null -s 1 -f track.raw

if [ $? -eq 0 ]; then
	exit 1
else
	rm -f track.raw
	exit 0
fi
