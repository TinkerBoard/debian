#!/bin/bash

echo "Stress Test of Reboot."

times=$(grep -r "reboot_times" /etc/reboot_times.txt | awk '{print $3}')

echo "########## Reboot device from test_Reboot.sh ##########"
((times+=1))
echo "reboot_times = "$times > /etc/reboot_times.txt
#systemctl reboot
sync
echo 1 | sudo tee /proc/sys/kernel/sysrq
echo b | sudo tee /proc/sysrq-trigger
