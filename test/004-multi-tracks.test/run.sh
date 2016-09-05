#!/bin/sh

# create large file
echo 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ > raw

echo -e '0:0:0\n0:1:0\n0:4:0\n0:10:0' | ${EXECUTABLE} -r 1 -c 1 -s 1 -i raw -n -track.raw


for track in {001..004}-track.raw ; do
	diff $track $track.expected >/dev/null
	if [ $? -eq 0 ] ; then
		rm $track
	else
		exit 1
	fi
done

rm raw
