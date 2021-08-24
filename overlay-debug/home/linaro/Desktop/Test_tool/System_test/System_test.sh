#!/bin/bash

version=1.6.1

log()
{
	logfile="/dev/kmsg"
	echo -e $1 | sudo tee $logfile
}

select_test_item()
{
	log "************************************************************"
	log ""
	log "         Tinker Board (S) System Test tool v_$version"
	log ""
	log  "************************************************************"
	log ""
	log "1. Start shutdown test"
	log "2. Start reboot test"
	log "3. Start suspend test"
	log "4. Stop test"
	log "5. Check test count"
	read -p "Select test case: " test_item
	log ""
}

info_view()
{
	log "*******************************************"
	log ""
	log "          $1 stress test start"
	log ""
	log "*******************************************"
	log "Reset test counter"
	sudo rm /etc/*_times.txt
}

pause(){
        read -n 1 -p "$*" INP
        if [ $INP != '' ] ; then
                echo -ne '\b \n'
        fi
}

if [ $1 ]; then
	test_item=$1
	path=$2
	source=win
else
	select_test_item
	path=$(pwd)/etc
	source=linux
fi

case $test_item in
	1)
		info_view Shutdown
		sudo cp $path/rc_shutdown.sh /etc/
		sudo cp $path/rc_shutdown.local /etc/rc.local
		sudo chmod 755 /etc/rc_shutdown.sh
		sudo chmod 755 /etc/rc.local
		sync
		sleep 5
		echo 1 | sudo tee /proc/sys/kernel/sysrq
		echo b | sudo tee /proc/sysrq-trigger
		;;
	2)
		info_view Reboot
		sudo cp $path/rc_reboot.sh /etc/
		sudo cp $path/rc_reboot.local /etc/rc.local
		sudo chmod 755 /etc/rc_reboot.sh
		sudo chmod 755 /etc/rc.local
		sync
		sleep 5
		echo 1 | sudo tee /proc/sys/kernel/sysrq
		echo b | sudo tee /proc/sysrq-trigger
		;;
	3)
		info_view Suspend
		times=0
		#echo performance | sudo tee $(find /sys/ -name *governor)
		while true; do
			sleep 10
			log "DUT try to suspend"
			sudo bash $path/suspend_test.sh
			log "DUT resume successfully"
			sleep 5
			((times+=1))
			log "suspend_times = "$times | sudo tee /etc/suspend_times.txt
		done
		;;
	4)
		log "Stop test, device will reboot again after 5 second"
		sudo cp $path/rc_stop.local /etc/rc.local
		sudo bash -c "./System_test.sh 5"
		sync
		sleep 5
		echo 1 | sudo tee /proc/sys/kernel/sysrq
		echo b | sudo tee /proc/sysrq-trigger
		;;
	5)
		if [ -f /etc/shutdown_times.txt ]; then
			cat /etc/shutdown_times.txt
		fi
		if [ -f /etc/reboot_times.txt ]; then
			cat /etc/reboot_times.txt
		fi
		if [ -f /etc/suspend_times.txt ]; then
			cat /etc/suspend_times.txt
		fi
		;;
	*)
		log "Unknown test case!"
		;;
esac

if [ $source == "linux" ]; then
	pause 'Press any key to exit...'
fi
