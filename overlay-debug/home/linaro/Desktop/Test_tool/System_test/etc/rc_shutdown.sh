#!/bin/bash

log()
{
	logfile="/dev/kmsg"
	echo -e $1 | sudo tee $logfile
}

log "Stress Test of Shutdown."

times=$(grep -r "shutdown_times" /etc/shutdown_times.txt | awk '{print $3}')

log "########## Shutdown device from test_shutdown.sh ##########"
((times+=1))
log "shutdown_times = "$times | sudo tee /etc/shutdown_times.txt
echo +40 > /sys/class/rtc/rtc0/wakealarm
#systemctl poweroff
sync
echo 1 | sudo tee /proc/sys/kernel/sysrq
echo o | sudo tee /proc/sysrq-trigger
