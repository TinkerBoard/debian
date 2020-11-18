#!/bin/bash

echo "Stress Test of Shutdown."

times=$(grep -r "shutdown_times" /etc/shutdown_times.txt | awk '{print $3}')

echo "########## Shutdown device from test_shutdown.sh ##########"
((times+=1))
echo "shutdown_times = "$times > /etc/shutdown_times.txt
echo +40 > /sys/class/rtc/rtc0/wakealarm
systemctl poweroff
