#!/bin/bash

log()
{
	logfile="/dev/kmsg"
	echo -e $1 | sudo tee $logfile
}

log "Stress Test of Reboot."

times=$(grep -r "reboot_times" /etc/reboot_times.txt | awk '{print $3}')

log "########## Reboot device from test_Reboot.sh ##########"
((times+=1))
log "reboot_times = "$times | sudo tee /etc/reboot_times.txt
#systemctl reboot
sync
echo 1 | sudo tee /proc/sys/kernel/sysrq
echo b | sudo tee /proc/sysrq-trigger
