#!/bin/sh

# create large file
dd if=/dev/urandom of=raw bs=1M count=1

echo 0:0:0 | ${EXECUTABLE} -r 44100 -c 1 -s 2 -i raw -n -track.raw

diff raw 001-track.raw >/dev/null && rm raw 001-track.raw
