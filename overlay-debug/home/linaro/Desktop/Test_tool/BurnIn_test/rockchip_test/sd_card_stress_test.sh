#!/bin/bash

#read -p "input source path:" src
#read -p "input destination path:" dest

sd_path=$(grep mmcblk1 /proc/partitions | awk '{print $4}' | head -n 1)
if [ -z $sd_path ]; then
        echo "The device dosen't have emmc!!"
        exit
fi

sd_blk=$(cat /proc/partitions | grep mmcblk0p1 | awk '{print $4}')
if [ -z $sd_blk ]; then
        echo "mmcblk0p1 not detect, check mmcblk0 exist or not!!"
        sd_blk=$(cat /proc/partitions | grep mmcblk0 | awk '{print $4}')
        if [ -z $sd_blk ]; then
                echo "SD card not detect, exit test!!"
                exit
        fi
fi

dev_name=/dev/$sd_blk

umount "$dev_name"
rm -rf /home/linaro/Desktop/sd
mkdir /home/linaro/Desktop/sd
mount -o rw "$dev_name" /home/linaro/Desktop/sd
sd_path=$(grep mmcblk0 /proc/mounts | cut -d ' ' -f 2)

if [ "$sd_path" != "/home/linaro/Desktop/sd" ]; then
        echo "SD card mount fail, exit test!!"
        exit
fi

src=/$sd_path/src
dest=/$sd_path/dest
emmc_src="$1/rockchip_test/src"

cp -vr $emmc_src /$sd_path/src

while [ 1 != 2 ]
do
    if [ -e ${dest} ]; then
        echo "the file is existed and remove it" && rm -vrf ${dest}
        sleep 1
	else
        cp -vr ${src} ${dest}
        sleep 1
    fi
done
