#!/bin/sh

# create large file
dd if=/dev/urandom of=raw bs=1M count=1

echo 0:0:0 | ${EXECUTABLE} -r 44100 -c 1 -s 2 -i raw -f track_%d.raw

diff raw track_1.raw >/dev/null && rm raw track_1.raw
