CUE/BIN Splitter
================
This is a tiny tool I wrote to take a slightly processed CUE file, a raw PCM file, and split it up
into a collection of raw PCM files (one for each track).

This tool takes a list of times (mm:ss:ff) from stdin and (blindly) outputs files named track_nnnn,
**It will overwrite any existing file with the same name**


Warning
-------
Try running `grep -i FIXME *.c`

**This will overwrite any existing file with the same name as an output file**


Sample Usage
------------
Assuming you want to use the first indices of each track as a boundary,

	grep "INDEX 01" cue-file | \
	  sed -e 's/INDEX 01//g' | \
	  cue-bin-split raw-file channels samples-rate bytes-per-sample

will output a bunch of files track_0001 through track_nnnn.
You might then push them through ffmpeg or something to get them to another format.
