#!/bin/bash

version=1.6.1

select_test_item()
{
	echo "************************************************************"
	echo
	echo "         Tinker Board (S) System Test tool v_$version"
	echo
	echo "************************************************************"
	echo
	echo "1. Start shutdown test"
	echo "2. Start reboot test"
	echo "3. Start suspend test"
	echo "4. Stop test"
	echo "5. Check test count"
	read -p "Select test case: " test_item
	echo
}

info_view()
{
	echo "*******************************************"
	echo
	echo "          $1 stress test start"
	echo
	echo "*******************************************"
	echo "Reset test counter"
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
		echo ""
		sudo bash -c "echo +20 > /sys/class/rtc/rtc0/wakealarm"
		sleep 5
		sudo systemctl poweroff
		;;
	2)
		info_view Reboot
		sudo cp $path/rc_reboot.sh /etc/
		sudo cp $path/rc_reboot.local /etc/rc.local
		sudo chmod 755 /etc/rc_reboot.sh
		sudo chmod 755 /etc/rc.local
		echo ""
		sleep 5
		sudo systemctl reboot
		;;
	3)
		info_view Suspend
		times=0
		#echo performance | sudo tee $(find /sys/ -name *governor)
		while true; do
			sleep 10
			sudo bash $path/suspend_test.sh
			sleep 5
			((times+=1))
			echo "suspend_times = "$times | sudo tee /etc/suspend_times.txt
		done
		;;
	4)
		echo "Stop test, device will reboot again after 5 second"
		sudo cp $path/rc_stop.local /etc/rc.local
		sudo bash -c "./System_test.sh 5"
		sleep 5
		sudo systemctl reboot
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
		echo "Unknown test case!"
		;;
esac

if [ $source == "linux" ]; then
	pause 'Press any key to exit...'
fi
