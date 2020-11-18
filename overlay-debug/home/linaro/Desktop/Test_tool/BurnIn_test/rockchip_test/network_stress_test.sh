#!/bin/bash

if ping -c1 -w5 8.8.8.8 >/dev/null 2>&1
then
    echo "Ping responded; Start Wi-Fi test!" >&2
else
    echo "Ping did not respond; " >&2
	echo "Please make sure Wi-Fi connected and device can access internet, then restart test again" >&2
	exit
fi

while [ 1 != 2 ]
do
	curl -s https://raw.githubusercontent.com/sivel/speedtest-cli/master/speedtest.py | python -
done

