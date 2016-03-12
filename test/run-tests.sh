#!/bin/sh

err_file="stderr.tmp"
out_file="stdout.tmp"
EXECUTABLE="$PWD/../cue-bin-split"

fail()
{
	echo "[FAIL] $i: $@"
	exit 1
}

for i in *.test ; do
	pushd ${i} >/dev/null
	( . ./run.sh ) 2>${err_file} >${out_file}
	if [ $? -ne 0 ] ; then
		fail "script had non-zero return code"
	fi

	if [ -f stderr.expected ] ; then
		diff stderr.expected stderr.tmp
		if [ $? -ne 0 ] ; then
			fail "stderr didn't match expected"
		fi
	fi

	if [ -f stdout.expected ] ; then
		diff stdout.expected stdout.tmp
		if [ $? -ne 0 ] ; then
			fail "stdout didn't match expected"
		fi
	fi

	echo "[PASS] $i"

	rm ${err_file} ${out_file}
	popd >/dev/null
done
