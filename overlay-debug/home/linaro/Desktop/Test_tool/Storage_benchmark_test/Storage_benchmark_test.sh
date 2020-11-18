#!/bin/bash

version=1.0

info_view()
{
	echo "************************************************"
	echo
	echo                "Storage Benchmark Test v_$version"
	echo
	echo "************************************************"
	echo
	read -p "Enter test path: " test_path
	echo
}

pause(){
        read -n 1 -p "$*" INP
        if [ $INP != '' ] ; then
                echo -ne '\b \n'
        fi
}

SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`
info_view

echo "Start Write Test"
dd if=/dev/zero of=$test_path/tmpfile bs=256M count=2 conv=fdatasync

echo "Start Read Test"
echo 3 | sudo tee /proc/sys/vm/drop_caches
dd if=$test_path/tmpfile of=/dev/null bs=256M count=2
rm $test_path/tmpfile

pause 'Press any key to exit...'
