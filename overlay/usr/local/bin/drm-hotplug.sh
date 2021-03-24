#!/bin/sh -x

# Try to figure out XAUTHORITY and DISPLAY
for pid in $(pgrep X 2>/dev/null || ls /proc|grep -ow "[0-9]*"|sort -rn); do
    PROC_DIR=/proc/$pid

    # Filter out non-X processes
    readlink $PROC_DIR/exe|grep -qwE "X$|Xorg$" || continue

    # Parse auth file and display from cmd args
    export XAUTHORITY=$(cat $PROC_DIR/cmdline|tr '\0' '\n'| \
        grep -w "\-auth" -A 1|tail -1)
    export DISPLAY=$(cat $PROC_DIR/cmdline|tr '\0' '\n'| \
        grep -w "^:.*" || echo ":0")

    echo Found auth: $XAUTHORITY for dpy: $DISPLAY
    break
done

export DISPLAY=${DISPLAY:-:0}

# Find an authorized user
unset USER
for user in root $(users);do
    sudo -u $user xdpyinfo &>/dev/null && \
        { USER=$user; break; }
done
[ $USER ] || exit 0

#config file locations for hdmi user config
FILE_HDMI_MODES="/boot/hdmi_user_config/hdmi_modes.txt"
FILE_HDMI_CONFIG="/boot/hdmi_user_config/hdmi_set_resolution.cfg"

HDMI="HDMI-1"
DP="DP-1"

HDMI_SYS="/sys/class/drm/card0-HDMI-A-1"
DP_SYS="/sys/class/drm/card0-DP-1"

HDMI_XRANDR_CONFIG="/boot/display/hdmi/xrandr.cfg"
DP_XRANDR_CONFIG="/boot/display/dp/xrandr.cfg"

HDMI_HOTPLUG_CONFIG="/boot/display/hdmi/hdmi_plug_flag.cfg"
DP_HOTPLUG_CONFIG="/boot/display/dp/dp_plug_flag.cfg"


HDMI_MODES_NODE="$HDMI_SYS/modes"
DP_MODES_NODE="$DP_SYS/modes"

HDMI_MODE_NODE="$HDMI_SYS/mode"
DP_MODE_NODE="$DP_SYS/mode"

hdmi_status=$(cat /sys/class/drm/card0-HDMI-A-1/status)
dp_status=$(cat /sys/class/drm/card0-DP-1/status)

#Save resolution if the external display is disconnected
#HDMI
if [ $hdmi_status = "disconnected" ]; then
	if [ -f $HDMI_HOTPLUG_CONFIG ]; then
		if [ "$(cat $HDMI_HOTPLUG_CONFIG)" != "Plug_Out" ]; then
			if [ "$(cat $HDMI_MODE_NODE)" != "" ]; then
				HDMI_SAVE_MODE="$(cat $HDMI_MODE_NODE |cut -d'p' -f1)"
				su $user -c "echo $HDMI_SAVE_MODE" > $HDMI_XRANDR_CONFIG
			fi
		fi
	fi
	sudo -u $user xrandr --output $HDMI --off
	su $user -c "echo Plug_Out" > $HDMI_HOTPLUG_CONFIG
fi

#DP
if [ $dp_status = "disconnected" ]; then
	if [ -f $DP_HOTPLUG_CONFIG ]; then
		if [ "$(cat $DP_HOTPLUG_CONFIG)" != "Plug_Out" ]; then
			if [ "$(cat $DP_MODE_NODE)" != "" ]; then
				DP_SAVE_MODE="$(cat $DP_MODE_NODE |cut -d'p' -f1)"
				su $user -c "echo $DP_SAVE_MODE" > $DP_XRANDR_CONFIG
			fi
		fi
	fi
	sudo -u $user xrandr --output $DP --off
	su $user -c "echo Plug_Out" > $DP_HOTPLUG_CONFIG
fi

#Restore external display preview resolution
#HDMI
sudo -u $user xrandr
if [ $hdmi_status = "connected" ]; then
	if [ "$(cat $FILE_HDMI_CONFIG)" = "" ]; then
		if [ -f $HDMI_HOTPLUG_CONFIG ]; then
			if [ "$(cat $HDMI_HOTPLUG_CONFIG)" = "Plug_Out" ]; then
				if [ -f $HDMI_XRANDR_CONFIG ]; then
					if grep -q $(cat $HDMI_XRANDR_CONFIG) $HDMI_MODES_NODE; then
						sudo -u $user xrandr --output $HDMI --mode $(cat $HDMI_XRANDR_CONFIG)
					else
						sudo -u $user xrandr --output $HDMI --auto
					fi
				else
					sudo -u $user xrandr --output $HDMI --auto
				fi
			else
				if [ "$(cat $HDMI_MODE_NODE)" = "" ]; then
					sudo -u $user xrandr --output $HDMI --auto
				fi
			fi
		fi
	else
		sudo -u $user xrandr --output $HDMI --mode $(cat $FILE_HDMI_CONFIG)
	fi
	su $user -c "echo Plug_In" > $HDMI_HOTPLUG_CONFIG

	#export HDMI modes to a text file
	sudo -u $user xrandr -q > $FILE_HDMI_MODES
	cat /sys/kernel/debug/clk/clk_summary | grep -ie npll -ie vop0 >> $FILE_HDMI_MODES

fi

#DP
if [ $dp_status = "connected" ]; then
	if [ -f $DP_HOTPLUG_CONFIG ]; then
		if [ "$(cat $DP_HOTPLUG_CONFIG)" = "Plug_Out" ]; then
			if [ -f $DP_XRANDR_CONFIG ]; then
				if grep -q $(cat $DP_XRANDR_CONFIG) $DP_MODES_NODE; then
					sudo -u $user xrandr --output $DP --mode $(cat $DP_XRANDR_CONFIG)
				else
					sudo -u $user xrandr --output $DP --auto
				fi
			else
				sudo -u $user xrandr --output $DP --auto
			fi
		else
			if [ "$(cat $DP_MODE_NODE)" = "" ]; then
				sudo -u $user xrandr --output $DP --auto
			fi
		fi
	fi
	su $user -c "echo Plug_In" > $DP_HOTPLUG_CONFIG
fi

# The DP needs reinit every time, so turn it off when disconnected
SYS="/sys/class/drm/card*-DP-1/"
grep -q dis $SYS/status && grep -q "" $SYS/mode && \
    sudo -u $user xrandr --output DP-1 --off

exit 0
