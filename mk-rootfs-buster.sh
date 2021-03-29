#!/bin/bash -e

# Directory contains the target rootfs
TARGET_ROOTFS_DIR="binary"

if [ -e $TARGET_ROOTFS_DIR ]; then
	sudo rm -rf $TARGET_ROOTFS_DIR
fi

if [ "$ARCH" == "armhf" ]; then
	ARCH='armhf'
elif [ "$ARCH" == "arm64" ]; then
	ARCH='arm64'
else
    echo -e "\033[36m please input is: armhf or arm64...... \033[0m"
fi

if [ ! $VERSION ]; then
	VERSION="debug"
fi

# Initialized to "eng", however this should be set in build.sh
if [ ! $VERSION_NUMBER ]; then
	VERSION_NUMBER="eng"
fi

if [ ! -e linaro-buster-alip-*.tar.gz ]; then
	echo "\033[36m Run mk-base-debian.sh first \033[0m"
fi

finish() {
	sudo umount $TARGET_ROOTFS_DIR/dev
	exit -1
}
trap finish ERR

echo -e "\033[36m Extract image \033[0m"
sudo tar -xpf linaro-buster-alip-*.tar.gz

# packages folder
sudo mkdir -p $TARGET_ROOTFS_DIR/packages
sudo cp -rf packages/$ARCH/* $TARGET_ROOTFS_DIR/packages

# overlay folder
sudo cp -rf overlay/* $TARGET_ROOTFS_DIR/

# overlay-firmware folder
sudo cp -rf overlay-firmware/* $TARGET_ROOTFS_DIR/

# overlay-debug folder
# adb, video, camera  test file
if [ "$VERSION" == "debug" ]; then
sudo cp -rf overlay-debug/* $TARGET_ROOTFS_DIR/
fi

## hack the serial
sudo cp -f overlay/usr/lib/systemd/system/serial-getty@.service $TARGET_ROOTFS_DIR/lib/systemd/system/serial-getty@.service

# adb
if [ "$ARCH" == "armhf" ] && [ "$VERSION" == "debug" ]; then
	sudo cp -rf overlay-debug/usr/local/share/adb/adbd-32 $TARGET_ROOTFS_DIR/usr/local/bin/adbd
elif [ "$ARCH" == "arm64"  ]; then
	sudo cp -rf overlay-debug/usr/local/share/adb/adbd-64 $TARGET_ROOTFS_DIR/usr/local/bin/adbd
fi

# bt/wifi firmware
if [ "$ARCH" == "armhf" ]; then
    sudo cp overlay-firmware/usr/bin/brcm_patchram_plus1_32 $TARGET_ROOTFS_DIR/usr/bin/brcm_patchram_plus1
    sudo cp overlay-firmware/usr/bin/rk_wifi_init_32 $TARGET_ROOTFS_DIR/usr/bin/rk_wifi_init
elif [ "$ARCH" == "arm64" ]; then
    sudo cp overlay-firmware/usr/bin/brcm_patchram_plus1_64 $TARGET_ROOTFS_DIR/usr/bin/brcm_patchram_plus1
    sudo cp overlay-firmware/usr/bin/rk_wifi_init_64 $TARGET_ROOTFS_DIR/usr/bin/rk_wifi_init
fi
sudo mkdir -p $TARGET_ROOTFS_DIR/system/lib/modules/
#sudo find ../kernel/drivers/net/wireless/rockchip_wlan/*  -name "*.ko" | \
#    xargs -n1 -i sudo cp {} $TARGET_ROOTFS_DIR/system/lib/modules/
# ASUS: Change to copy all the kernel modules built from build.sh.
sudo cp -rf  lib_modules/lib/modules $TARGET_ROOTFS_DIR/lib/

# glmark2
sudo rm -rf $TARGET_ROOTFS_DIR/usr/local/share/glmark2
sudo mkdir -p $TARGET_ROOTFS_DIR/usr/local/share/glmark2
if [ "$ARCH" == "armhf" ] && [ "$VERSION" == "debug" ]; then
	sudo cp -rf overlay-debug/usr/local/share/glmark2/armhf/share/* $TARGET_ROOTFS_DIR/usr/local/share/glmark2
	sudo cp overlay-debug/usr/local/share/glmark2/armhf/bin/glmark2-es2 $TARGET_ROOTFS_DIR/usr/local/bin/glmark2-es2
elif [ "$ARCH" == "arm64" ] && [ "$VERSION" == "debug" ]; then
	sudo cp -rf overlay-debug/usr/local/share/glmark2/aarch64/share/* $TARGET_ROOTFS_DIR/usr/local/share/glmark2
	sudo cp overlay-debug/usr/local/share/glmark2/aarch64/bin/glmark2-es2 $TARGET_ROOTFS_DIR/usr/local/bin/glmark2-es2
fi

echo -e "\033[36m Change root.....................\033[0m"
if [ "$ARCH" == "armhf" ]; then
	sudo cp /usr/bin/qemu-arm-static $TARGET_ROOTFS_DIR/usr/bin/
elif [ "$ARCH" == "arm64"  ]; then
	sudo cp /usr/bin/qemu-aarch64-static $TARGET_ROOTFS_DIR/usr/bin/
fi

# Utilize the nameserver configuration from the host.
# This will be reset back to the original one in the end.
sudo cp -b /etc/resolv.conf $TARGET_ROOTFS_DIR/etc/resolv.conf

sudo mount -o bind /dev $TARGET_ROOTFS_DIR/dev

cat << EOF | sudo chroot $TARGET_ROOTFS_DIR

apt-get update

chmod o+x /usr/lib/dbus-1.0/dbus-daemon-launch-helper
chmod +x /etc/rc.local

#---------------power management --------------
# The following packages are included in the base system.
#apt-get install -y busybox pm-utils triggerhappy
cp /etc/Powermanager/triggerhappy.service  /lib/systemd/system/triggerhappy.service

#---------------system--------------
# The following packages are included in the base system.
#apt-get install -y git fakeroot devscripts cmake binfmt-support dh-make dh-exec pkg-kde-tools device-tree-compiler \
#bc cpio parted dosfstools mtools libssl-dev dpkg-dev isc-dhcp-client-ddns
#apt-get install -f -y

#---------------Rga--------------
dpkg -i /packages/rga/*.deb

echo -e "\033[36m Setup Video.................... \033[0m"
# The following packages are included in the base system.
#apt-get install -y gstreamer1.0-plugins-bad gstreamer1.0-plugins-base gstreamer1.0-tools gstreamer1.0-alsa \
#gstreamer1.0-plugins-base-apps qtmultimedia5-examples
#apt-get install -f -y

dpkg -i  /packages/mpp/*
dpkg -i  /packages/gst-rkmpp/*.deb
#apt-mark hold gstreamer1.0-x
# Fix the installation in the end.
#apt-get install -f -y

#---------Camera---------
echo -e "\033[36m Install camera.................... \033[0m"
# The following packages are included in the base system.
#apt-get install cheese v4l-utils -y
dpkg -i  /packages/rkisp/*.deb
dpkg -i  /packages/libv4l/*.deb

#---------Xserver---------
echo -e "\033[36m Install Xserver.................... \033[0m"
#apt-get build-dep -y xorg-server-source
# The following packages are included in the base system.
#apt-get install -y libgl1-mesa-dev libgles1 libgles1 libegl1-mesa-dev libc-dev-bin libc6-dev libfontenc-dev libfreetype6-dev \
#libpciaccess-dev libpng-dev libpng-tools libxfont-dev libxkbfile-dev linux-libc-dev manpages manpages-dev xserver-common zlib1g-dev \
#libdmx1 libpixman-1-dev libxcb-xf86dri0 libxcb-xv0
#apt-get install -f -y

dpkg -i /packages/xserver/*.deb
# Fix the installation in the end.
#apt-get install -f -y
# apt-mark hold xserver-common xserver-xorg-core xserver-xorg-legacy

#---------------Openbox--------------
echo -e "\033[36m Install openbox.................... \033[0m"
# The following package is included in the base system.
#apt-get install -y openbox
dpkg -i  /packages/openbox/*.deb
# Fix the installation in the end.
#apt-get install -f -y

#------------------pcmanfm------------
echo -e "\033[36m Install pcmanfm.................... \033[0m"
# The following package is included in the base system.
#apt-get install -y pcmanfm
dpkg -i  /packages/pcmanfm/*.deb
# Fix the installation in the end.
#apt-get install -f -y

#------------------ffmpeg------------
echo -e "\033[36m Install ffmpeg.................... \033[0m"
# The following package is included in the base system.
#apt-get install -y ffmpeg
dpkg -i  /packages/ffmpeg/*.deb
# Fix the installation in the end.
#apt-get install -f -y

#------------------mpv------------
# Don't install mpv since we don't have the license.
#echo -e "\033[36m Install mpv.................... \033[0m"
#apt-get install -y libmpv1 mpv
#dpkg -i  /packages/mpv/*.deb
#apt-get install -f -y

#---------update chromium-----
# The following package is included in the base system.
#apt-get install -y chromium
apt-get install -f -y /packages/chromium/*.deb

#------------------libdrm------------
echo -e "\033[36m Install libdrm.................... \033[0m"
dpkg -i  /packages/libdrm/*.deb
#apt-get install -f -y

# mark package to hold
# apt-mark hold libv4l-0 libv4l2rds0 libv4lconvert0 libv4l-dev v4l-utils
#apt-mark hold librockchip-mpp1 librockchip-mpp-static librockchip-vpu0 rockchip-mpp-demos
#apt-mark hold xserver-common xserver-xorg-core xserver-xorg-legacy
#apt-mark hold libegl-mesa0 libgbm1 libgles1 alsa-utils
#apt-get install -f -y

#---------------Custom Script--------------
systemctl mask systemd-networkd-wait-online.service
systemctl mask NetworkManager-wait-online.service
rm /lib/systemd/system/wpa_supplicant@.service

#-------ASUS customization start-------
if [ "$VERSION" == "debug" ] ; then
    # Enable test.service to change the owner for the test tools.
    systemctl enable test.service
fi

ln -s /lib/systemd/system/hciuart.service /etc/systemd/system/multi-user.target.wants/hciuart.service

#-------------blueman--------------
bash /etc/init.d/blueman.sh
rm /etc/init.d/blueman.sh

#-------------mount partition p7--------------
systemctl enable mountboot.service

#--------------Audio--------------
chmod 755 /etc/pulse/movesinks.sh
chmod 755 /etc/audio/jack_auto_switch.sh
chmod 755 /etc/audio/jack_switch_at_boot.sh
ls -s /lib/systemd/system/jack-switch-at-boot.service /etc/systemd/system/multi-user.target.wants/jack-switch-at-boot.service

#--------------Wi-Fi--------------
ln -s /lib/systemd/system/wifi.service /etc/systemd/system/multi-user.target.wants/wifi.service

#--------------voltage-detect--------------
ln -s /lib/systemd/system/voltage-detect.service /etc/systemd/system/multi-user.target.wants/voltage-detect.service
chmod 775 /etc/init.d/voltage-detect.py

plymouth-set-default-theme script

# Don't show this on menu for now since it does not work.
rm /usr/share/applications/squeak.desktop

# With the packages xfonts-100dpi and xfonts-75dpi installed, this is to avoid warning when opening xkeycaps.
xset +fp /usr/share/fonts/X11/75dpi/
xset +fp /usr/share/fonts/X11/100dpi/

# Clean up and fix the packages installation.
apt-get install -f -y
apt autoremove -y

echo $VERSION_NUMBER > /etc/version
#-------ASUS customization end-------

#---------------Clean--------------
rm -rf /var/lib/apt/lists/*

EOF

sudo umount $TARGET_ROOTFS_DIR/dev

# Reset resolve.conf to the original one.
sudo mv $TARGET_ROOTFS_DIR/etc/resolv.conf~ $TARGET_ROOTFS_DIR/etc/resolv.conf
