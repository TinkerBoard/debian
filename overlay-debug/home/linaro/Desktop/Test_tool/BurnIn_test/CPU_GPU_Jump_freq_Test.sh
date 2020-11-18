#!/bin/bash -e

if [ $1 ];then
	SleepTime=$1
else
	SleepTime=1
fi

a=0

while [ $a -lt 100000000 ]
do
	echo "times: $a"
	echo "==============================="
	

	sudo su -c "echo userspace > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
	sudo su -c "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"

	sudo su -c "echo userspace > /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor"
	sudo su -c "cat /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor"

	sudo su -c "echo userspace > /sys/class/devfreq/ff9a0000.gpu/governor"
	sudo su -c "cat /sys/class/devfreq/ff9a0000.gpu/governor"

	unset FREQS_SMALL
	read -a FREQS_SMALL < /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies

	unset FREQS_BIG
	read -a FREQS_BIG < /sys/devices/system/cpu/cpu4/cpufreq/scaling_available_frequencies

	unset FREQS_GPU
	read -a FREQS_GPU < /sys/class/devfreq/ff9a0000.gpu/available_frequencies
	
	
	FREQ=${FREQS_SMALL[$a % ${#FREQS_SMALL[@]} ]}
	sudo su -c "echo ${FREQ} > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed"
	sudo su -c "cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"

	FREQ=${FREQS_BIG[$a % ${#FREQS_BIG[@]} ]}
	sudo su -c "echo ${FREQ} > /sys/devices/system/cpu/cpu4/cpufreq/scaling_setspeed"
	sudo su -c "cat /sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq"

	FREQ=${FREQS_GPU[$a % ${#FREQS_GPU[@]} ]}
	sudo su -c "echo 800000000 > /sys/class/devfreq/ff9a0000.gpu/max_freq"
	sudo su -c "echo ${FREQ} > /sys/class/devfreq/ff9a0000.gpu/min_freq"
	sudo su -c "echo ${FREQ} > /sys/class/devfreq/ff9a0000.gpu/max_freq"
	sudo su -c "cat /sys/class/devfreq/ff9a0000.gpu/cur_freq"

	sleep ${SleepTime}

	a=`expr $a + 1`
	echo "================================"

done



