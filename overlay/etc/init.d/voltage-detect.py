#!/usr/bin/python
### BEGIN INIT INFO
# Provides:          ASUS
# Required-Start:
# Required-Stop:
# Default-Start:
# Default-Stop:
# Short-Description:
# Description:       5V voltage detect
### END INIT INFO

import gi
gi.require_version('Notify', '0.7')
from gi.repository import GLib, Notify


DETECT_VOLTAGE = 4.65 #4.65
ADC_IN2_RAW_PATH = '/sys/bus/iio/devices/iio:device0/in_voltage2_raw'

DETECT_ON = True

ICON_PATH = '/usr/share/pixmaps/Plug-04.png'
TITLE = 'Low Voltage'
MSG = 'The system may turn off due to low power input (input voltage below 4.65V), when this happens, please disconnect high power consuming peripherals or change to a qualified power supply.'

SHOW_MORE = False

def BoardInfo():
    with open('/proc/boardinfo') as board_info:
        board = board_info.readline().strip()
    return board

class App():
    def __init__(self):

        Notify.init("voltage-detect")

        self.notification = Notify.Notification.new(
            TITLE,
            MSG,
            ICON_PATH
        )
        self.notification.set_urgency(Notify.Urgency.CRITICAL)
        self.notification.set_timeout(2)
        self.notification.add_action (
            "action_click",
            "Less info",
            self.notification_callback,
            None
        )

        if DETECT_ON :
            self.check()
        else:
            exit()

    def show(self):
        self.notification.clear_actions()
        if SHOW_MORE:
            self.notification.update(TITLE, MSG, ICON_PATH)
            self.notification.add_action (
                "action_click",
                "Hide more",
                self.notification_callback,
                None
            )
        else:
            self.notification.update(TITLE, "", ICON_PATH)
            self.notification.add_action (
                "action_click",
                "Show more",
                self.notification_callback,
                None
            )
        self.notification.show()

    def check(self):
        with open(ADC_IN2_RAW_PATH) as in_voltage2_raw:
            val2_raw = int(in_voltage2_raw.readline())

        val_input = float(val2_raw / ((82.0/302.0) * 1023.0 / 1.8)) + 0.1
        if val_input < DETECT_VOLTAGE :
            self.show()
        else:
            self.notification.close()

        GLib.timeout_add_seconds(1, self.check)

    def notification_callback(self, notification, action_name, data):
        global SHOW_MORE
        SHOW_MORE = not SHOW_MORE

app = App()
GLib.MainLoop().run()
