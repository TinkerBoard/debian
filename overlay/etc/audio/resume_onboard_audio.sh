#!/bin/bash

function check_modules() {
	bt_output=`$ENV pactl list short modules | grep BT_VOIP-Output | cut -f1 -d$'\t'`
	spdif_output=`$ENV pactl list short modules | grep SPDIF-Output | cut -f1 -d$'\t'`
	headset_output=`$ENV pactl list short modules | grep Headset-Output | cut -f1 -d$'\t'`
	bt_input=`$ENV pactl list short modules | grep BT_VOIP-Input | cut -f1 -d$'\t'`
	headset_input=`$ENV pactl list short modules | grep Headset-Input | cut -f1 -d$'\t'`

	if [ -n "$bt_output" ] && [ -n "$spdif_output" ] && [ -n "$headset_output" ] && [ -n "$bt_input" ] && [ -n "$headset_input" ]; then
		$ENV pasuspender true
		exit 0
	fi
}

function reload_modules() {
	$ENV pactl unload-module $bt_output
	$ENV pactl unload-module $spdif_output
	$ENV pactl unload-module $headset_output
	$ENV pactl unload-module $bt_input
	$ENV pactl unload-module $headset_input
	$ENV pacmd load-module module-alsa-sink device=OnBoard_D0 sink_properties=device.description="BT_VOIP-Output"
	$ENV pacmd load-module module-alsa-sink device=OnBoard_D1 sink_properties=device.description="SPDIF-Output"
	$ENV pacmd load-module module-alsa-sink device=OnBoard_D2 sink_properties=device.description="Headset-Output"
	$ENV pacmd load-module module-alsa-source device=OnBoard_D0 source_properties=device.description="BT_VOIP-Input"
	$ENV pacmd load-module module-alsa-source device=OnBoard_D1 source_properties=device.description="Headset-Input"
}

ENV="sudo -u linaro PULSE_RUNTIME_PATH=/run/user/1000/pulse"

check_modules
reload_modules
check_modules

card=`$ENV pactl list short modules | grep usb-Generic_USB_Audio | cut -f1 -d$'\t'`
while [ -z "$card" ]; do
	card=`$ENV pactl list short modules | grep usb-Generic_USB_Audio | cut -f1 -d$'\t'`
done

$ENV pactl unload-module $card
reload_modules

gpio=`cat /sys/class/gpio/gpio220/value`
if [ ${gpio} == 1 ]; then
	/bin/bash /etc/audio/jack_auto_switch.sh "out"
fi
if [ ${gpio} == 0 ]; then
	/bin/bash /etc/audio/jack_auto_switch.sh "in"
fi

$ENV pacmd set-default-source alsa_input.OnBoard_D1

$ENV pasuspender true
