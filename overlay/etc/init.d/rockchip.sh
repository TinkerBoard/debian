#!/bin/bash -e
### BEGIN INIT INFO
# Provides:          rockchip
# Required-Start:
# Required-Stop:
# Default-Start:
# Default-Stop:
# Short-Description:
# Description:       Setup rockchip platform environment
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
function link_mali() {
if [ "$1" == "rk3288" ];
then
    GPU_VERSION=$(cat /sys/devices/platform/*gpu/gpuinfo)
    if echo $GPU_VERSION|grep -q r1p0;
    then
        dpkg -i  /packages/libmali/libmali-rk-midgard-t76x-r18p0-r1p0_*.deb #3288w
    else
        dpkg -i  /packages/libmali/libmali-rk-midgard-t76x-r18p0-r0p0_*.deb
    fi
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
elif [[  "$1" == "rk3328"  ]]; then
    dpkg -i  /packages/libmali/libmali-rk-utgard-450-*.deb
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
    mv /etc/X11/xorg.conf.d/20-modesetting.conf /etc/X11/xorg.conf.d/20-modesetting.conf.backup
    mv /etc/X11/xorg.conf.d/20-armsoc.conf.backup /etc/X11/xorg.conf.d/20-armsoc.conf
elif [[  "$1" == "rk3399"  ]]; then
    dpkg -i  /packages/libmali/libmali-rk-midgard-t86x-*.deb
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
elif [[  "$1" == "rk3399pro"  ]]; then
    dpkg -i  /packages/libmali/libmali-rk-midgard-t86x-*.deb
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
elif [[  "$1" == "rk3326"  ]]; then
    dpkg -i  /packages/libmali/libmali-rk-bifrost-g31-*.deb
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
elif [[  "$1" == "px30"  ]]; then
    dpkg -i  /packages/libmali/libmali-rk-bifrost-g31-*.deb
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
elif [[  "$1" == "rk3128"  ]]; then
    dpkg -i  /packages/libmali/libmali-rk-utgard-400-*.deb
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
elif [[  "$1" == "rk3036"  ]]; then
    dpkg -i  /packages/libmali/libmali-rk-utgard-400-*.deb
    dpkg -i  /packages/libmali/libmali-rk-dev_*.deb
    mv /etc/X11/xorg.conf.d/20-modesetting.conf /etc/X11/xorg.conf.d/20-modesetting.conf.backup
    mv /etc/X11/xorg.conf.d/20-armsoc.conf.backup /etc/X11/xorg.conf.d/20-armsoc.conf
    # sed -i -e 's:"SWcursor"              "false":"SWcursor"              "true":' \
    #     -i /etc/X11/xorg.conf.d/20-armsoc.conf
fi
if [ -e "/usr/lib/aarch64-linux-gnu" ]; then
    cd /usr/lib/aarch64-linux-gnu/
    if [ -e "libEGL.so.1.1.0" ]; then
        rm libEGL.so.1.1.0
    elif [ -e "libGLESv2.so.2.0.0" ]; then
	rm libGLESv2.so.2.0.0
    elif [ -e "libGLEW.so.2.0.0" ]; then
	rm libGLEW.so.2.0.0
    fi
    ln -sf libMali.so libEGL.so.1.1.0
    ln -sf libMali.so libEGL.so
    ln -sf libMali.so libEGL.so.1.0.0
    ln -sf libMali.so libEGL.so.1.4
    ln -sf libMali.so libGLESv2.so
    ln -sf libMali.so libGLESv2.so.2.0
    ln -sf libMali.so libGLESv2.so.2.0.0
    ln -sf libMali.so libGLESv1_CM.so
    ln -sf libMali.so libGLESv1_CM.so.1
    ln -sf libMali.so libGLESv1_CM.so.1.1

fi
if [ -e "/usr/lib/arm-linux-gnueabihf" ]; then
    cd /usr/lib/arm-linux-gnueabihf/
    if [ -e "libEGL.so.1.1.0" ]; then
        rm libEGL.so.1.1.0
    elif [ -e "libGLESv2.so.2.0.0" ]; then
	rm libGLESv2.so.2.0.0
    elif [ -e "libGLEW.so.2.0.0" ]; then
	rm libGLEW.so.2.0.0
    fi

    ln -sf libMali.so libEGL.so.1.1.0
    ln -sf libMali.so libEGL.so
    ln -sf libMali.so libEGL.so.1.0.0
    ln -sf libMali.so libEGL.so.1.4
    ln -sf libMali.so libGLESv2.so
    ln -sf libMali.so libGLESv2.so.2.0
    ln -sf libMali.so libGLESv2.so.2.0.0
    ln -sf libMali.so libGLESv1_CM.so
    ln -sf libMali.so libGLESv1_CM.so.1
    ln -sf libMali.so libGLESv1_CM.so.1.1
fi

}

function update_npu_fw() {
    /usr/bin/npu-image.sh
    sleep 1
    /usr/bin/npu_transfer_proxy&
}

COMPATIBLE=$(cat /proc/device-tree/compatible)
if [[ $COMPATIBLE =~ "rk3288" ]];
then
    CHIPNAME="rk3288"
elif [[ $COMPATIBLE =~ "rk3328" ]]; then
    CHIPNAME="rk3328"
elif [[ $COMPATIBLE =~ "rk3399" && $COMPATIBLE =~ "rk3399pro" ]]; then
    CHIPNAME="rk3399pro"
    update_npu_fw
elif [[ $COMPATIBLE =~ "rk3399" ]]; then
    CHIPNAME="rk3399"
elif [[ $COMPATIBLE =~ "rk3326" ]]; then
    CHIPNAME="rk3326"
elif [[ $COMPATIBLE =~ "px30" ]]; then
    CHIPNAME="px30"
elif [[ $COMPATIBLE =~ "rk3128" ]]; then
    CHIPNAME="rk3128"
else
    CHIPNAME="rk3036"
fi
COMPATIBLE=${COMPATIBLE#rockchip,}
BOARDNAME=${COMPATIBLE%%rockchip,*}

# first boot configure
if [ ! -e "/usr/local/first_boot_flag" ] ;
then
    echo "It's the first time booting."
    echo "The rootfs will be configured."

    # Force rootfs synced
    mount -o remount,sync /

    link_mali ${CHIPNAME}
    setcap CAP_SYS_ADMIN+ep /usr/bin/gst-launch-1.0
    rm -rf /packages

    # The base target does not come with lightdm
    systemctl restart lightdm.service || true

    sed /etc/dphys-swapfile -i -e "s/^#CONF_SWAPSIZE=.*/CONF_SWAPSIZE=100/"
    systemctl restart dphys-swapfile.service

    touch /usr/local/first_boot_flag

    systemctl restart voltage-detect.service
    systemctl restart jack-switch-at-boot.service
fi

# set act-led trigger function
cmdline=$(cat /proc/cmdline)
storage=`echo $cmdline|awk '{print match($0,"storagemedia=emmc")}'`;

if [ $storage -gt 0 ]; then
    #emmc
    echo mmc1 > /sys/class/leds/act-led/trigger
else
    #sdcard
    echo mmc0 > /sys/class/leds/act-led/trigger
fi

# enable adbd service
if [ -e "/etc/init.d/adbd.sh" ] ;
then
    cd /etc/rcS.d
    if [ ! -e "S01adbd.sh" ] ;
    then
        ln -s ../init.d/adbd.sh S01adbd.sh
    fi
    cd /etc/rc6.d
    if [ ! -e "K01adbd.sh" ] ;
    then
        ln -s ../init.d/adbd.sh K01adbd.sh
    fi

    service adbd.sh start
fi

# support power management
if [ -e "/usr/sbin/pm-suspend" ] ;
then
    mv /etc/Powermanager/power-key.sh /usr/bin/
    mv /etc/Powermanager/power-key.conf /etc/triggerhappy/triggers.d/
    if [[ "$CHIPNAME" == "rk3399pro" ]];
    then
        mv /etc/Powermanager/01npu /usr/lib/pm-utils/sleep.d/
        mv /etc/Powermanager/02npu /lib/systemd/system-sleep/
    fi
    mv /etc/Powermanager/triggerhappy /etc/init.d/triggerhappy

    rm /etc/Powermanager -rf
    service triggerhappy restart
fi

# read mac-address from efuse
# if [ "$BOARDNAME" == "rk3288-miniarm" ]; then
#     MAC=`xxd -s 16 -l 6 -g 1 /sys/bus/nvmem/devices/rockchip-efuse0/nvmem | awk '{print $2$3$4$5$6$7 }'`
#     ifconfig eth0 hw ether $MAC
# fi
