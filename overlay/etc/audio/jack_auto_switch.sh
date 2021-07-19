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

source /etc/audio/audio.conf
echo "jack_auto_switch=${jack_auto_switch}";
echo "jack_default_device=${jack_default_device}";

if [ "${jack_auto_switch}" == "on" ]; then

if [ "$1" == "in" ]; then
	echo "audio jack plug in";
	sudo -u linaro DISPLAY=:0.0 notify-send -t 3000 "Audio Jack Plug In"
	sudo -u linaro PULSE_RUNTIME_PATH=/run/user/1000/pulse pacmd set-default-sink "alsa_output.OnBoard_D2"
	sudo -u linaro PULSE_RUNTIME_PATH=/run/user/1000/pulse pacmd list-sink-inputs | grep index | while read line
	do
	sudo -u linaro PULSE_RUNTIME_PATH=/run/user/1000/pulse pacmd move-sink-input `echo $line | cut -f2 -d' '` "alsa_output.OnBoard_D2"
	done
fi

if [ "$1" == "out" ]; then
	echo "headphone plug out";
	sudo -u linaro DISPLAY=:0.0 notify-send -t 3000 "Audio Jack Plug Out"
	sudo -u linaro PULSE_RUNTIME_PATH=/run/user/1000/pulse pacmd set-default-sink ${jack_default_device}
	sudo -u linaro PULSE_RUNTIME_PATH=/run/user/1000/pulse pacmd list-sink-inputs | grep index | while read line
	do
	sudo -u linaro PULSE_RUNTIME_PATH=/run/user/1000/pulse pacmd move-sink-input `echo $line | cut -f2 -d' '` ${jack_default_device}
	done
fi

fi
