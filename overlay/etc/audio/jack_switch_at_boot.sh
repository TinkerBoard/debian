#!/bin/bash

projectid=`cat /proc/projectid`
boardid=`cat /proc/boardid`
echo project=${projectid};
echo boardid=${boardid};

if [ "${projectid}" == "7" ]; then
	if [ "${boardid}" == "0" ]; then
		exit 0
	elif [ "${boardid}" == "1" ]; then
		exit 0
	elif [ "${boardid}" == "2" ]; then
		exit 0
	fi
fi

gpio=`cat /sys/class/gpio/gpio220/value`
echo "gpio=${gpio}";

sleep 5
if [ ${gpio} == "1" ]; then
	echo "audio jack plug out at boot";
	/bin/bash /etc/audio/jack_auto_switch.sh "out"
fi

if [ ${gpio} == "0" ]; then
	echo "audio jack plug in at boot";
	/bin/bash /etc/audio/jack_auto_switch.sh "in"
fi
