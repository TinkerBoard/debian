#!/bin/bash

#disalbe thermal
echo "Setting CPU / GPU / DDR in highest performance mode"

#if [ -e /sys/class/thermal/thermal_zone0 ]; then
#  echo user_space >/sys/class/thermal/thermal_zone0/policy
#  echo disabled > /sys/class/thermal/thermal_zone0/mode
#  echo 0 > /sys/class/thermal/thermal_zone0/cdev0/cur_state
#  echo 0 > /sys/class/thermal/thermal_zone0/cdev1/cur_state
#  echo 0 > /sys/class/thermal/thermal_zone0/cdev2/cur_state

#fi

#if [ -e /sys/class/thermal/thermal_zone1 ]; then
#  echo user_space > /sys/class/thermal/thermal_zone1/policy
#  echo disabled > /sys/class/thermal/thermal_zone1/mode
#fi

echo "performance" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo "performance" > /sys/class/devfreq/ffa30000.gpu/governor
