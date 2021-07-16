#!/bin/bash

echo "Run audio_setting on first time boot"

lsusb | grep "0bda:481a"
if [ "$?" == "0" ]; then
	audio_codec="y"
else
	lsusb | grep "0bda:4040"
	if [ "$?" == "0" ]; then
		audio_codec="y"
	else
		lsusb | grep "0bda:4030"
		if [ "$?" == "0" ]; then
			audio_codec="y"
		else
			lsusb | grep "0bda:49f6"
			if [ "$?" == "0" ]; then
				audio_codec="y"
			else
				audio_codec="n"
				sudo cp /etc/audio/init_default.pa /etc/pulse/default.pa
				sudo cp /etc/audio/init_audio.conf /etc/audio/audio.conf
			fi
		fi
	fi
fi

echo "Audio_Codec = $audio_codec"
