#!/bin/bash

boardinfo=`cat /proc/boardinfo`
if [ "${boardinfo}" != "Tinker Board S" ] && [ "${boardinfo}" != "Tinker R/BR" ] && [ "${boardinfo}" != "Tinker Board S/HV" ] && [ "${boardinfo}" != "Tinker Board R2" ] && [ "${boardinfo}" != "Tinker Board S R2.0" ]; then
	exit 0
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
