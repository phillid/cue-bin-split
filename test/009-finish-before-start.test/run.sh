#!/bin/sh

dd if=/dev/urandom of=input.raw bs=1M count=1 2>/dev/null

echo -e '0:1:0\n0:0:0\n' | ${EXECUTABLE} -r 1 -c 1 -i input.raw -s 1 -n -track.raw

retval=$?
if [ $retval -eq 0 ]; then
	rm -f {001-track,input}.raw
fi

exit $retval
