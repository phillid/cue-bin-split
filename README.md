CUE/BIN Splitter
================
This is a tiny tool I wrote to take a slightly processed CUE file, a raw PCM file, and split it up
into a collection of raw PCM files (one for each track).

This tool takes a list of times (mm:ss:ff) from stdin and (blindly) outputs files named track_nnnn,
**It will overwrite any existing file with the same name**


Warning
-------
Try running `grep -i FIXME *.c`


Sample Usage
------------
Assuming you want to use the first indices of each track as a boundary,

	grep "INDEX 01" cue-file | \
	  sed -e 's/INDEX 01//g' | \
	  cue-bin-split raw-file channels samples-rate bytes-per-sample name-format

Where format is something like `track_%04d`.
This will output a bunch of files named accordingly.
You might then push them through ffmpeg or something to get them to another audio format.
