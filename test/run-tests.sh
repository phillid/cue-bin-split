#!/bin/sh

EXECUTABLE="$PWD/../cue-bin-split"

fail()
{
	echo "[FAIL] $i: $@"
	exit 1
}

check_expected()
{
	[ -z $1 ] && echo WARN: check_expected called with no argument
	if [ -f $1.expected ] ; then
		diff $1.expected $1.tmp >/dev/null
		if [ $? -ne 0 ] ; then
			fail "$1 didn't match expected"
		fi
	fi
}

for i in *.test ; do
	pushd ${i} >/dev/null
	( . ./run.sh ) 2>stderr.tmp >stdout.tmp
	if [ $? -ne 0 ] ; then
		fail "script had non-zero return code"
	fi

	check_expected stdout
	check_expected stderr

	echo "[PASS] $i"

	rm std{err,out}.tmp
	popd >/dev/null
done
