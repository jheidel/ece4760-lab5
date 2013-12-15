import sys, logging, signal
import argparse
from threading import Thread, Event
from time import sleep
from gi.repository import Gtk, Gdk, GLib

from pylib.ildaparser import IldaParser
from pylib.ildaframe import IldaFrame
from pylib.laserviz import LaserViz
from pylib.serialcomm import SerialComm
from pylib.log import init_logging, with_logging

parser = argparse.ArgumentParser(description='Laser Projector Software')
parser.add_argument('port', nargs='?', help="Path to the serial port connected to the laser projector")
parser.add_argument('filename', nargs='?', help="Path to the ILDA file you wish to display")
parser.add_argument('--square', choices=['x', 'y'], help="Generate a square wave in the indicated axis") 
args = parser.parse_args()

if args.filename is None and args.square is None:
    print "Must select either an ILDA file or indicate test mode using --square"
    parser.print_help()
    sys.exit()

if args.square is not None:
    test_frame = True
    p = None
else:
    test_frame = False
    p = IldaParser(args.filename)

port = args.port

class FramePlayer(Thread):
    def __init__(self, lv, p, ser):
        Thread.__init__(self)
        self.lv = lv
        self.p = p
        self.kill = Event()
        self.FPS = 10
        self.ser = ser
        self.start()

    def stop(self):
        self.kill.set()

    def run(self):
        while True:
            if test_frame:
                f = IldaFrame.SqWaveTestPattern(x=(args.square=="x"))
                self.lv.set_frame(f)
                if self.ser is not None:
                    self.ser.set_frame(f)
                return
            for f in self.p.get_frames():
                self.lv.set_frame(f)
                if self.ser is not None:
                    self.ser.set_frame(f)
                self.kill.wait(1.0/self.FPS)
                if (self.kill.is_set()):
                    return

@with_logging
class MainGUI:

    def window_destroy(self, event):
        Gtk.main_quit()
        self.player.stop()
        if self.serial is not None:
            self.serial.stop()
        self.log.info("Bye!")

    def new_pps_selected(self, event):
        if self.serial is not None:
            self.serial.set_pps(int(self.ppsspinner.get_value()))

    def __init__(self):
        self.gladefile = "pylib/gui.glade"

        self.builder = Gtk.Builder()
        self.builder.add_from_file(self.gladefile)
        self.builder.connect_signals(self)

        self.laserviz = LaserViz(self)

        self.window = self.builder.get_object("mainWindow")
        self.window.show()

        self.ppsspinner = self.builder.get_object("spinbutton1")

        if port is not None:
            self.serial = SerialComm(port)
            if p is not None:
                self.serial.set_frame(p.get_initial_frame())
            self.serial.start()
        else:
            self.log.warning("No serial port specified. Operating in view-only mode.")
            self.serial = None

        self.player = FramePlayer(self.laserviz, p, self.serial)

        #Ctrl+C handling
        def handler(signum, frame):
            self.log.warning("INTERRUPT; shutting down...")
            self.window_destroy(None)
        signal.signal(signal.SIGTERM, handler)
        signal.signal(signal.SIGINT, handler)

        #This fairly pointless function is necessary to periodically wake up
        #the gtk main thread to detect system interrupts even when not focused
        #I believe this is due to a Gtk bug
        GLib.timeout_add(500, lambda : True)
        
        Gdk.threads_init()
        GLib.threads_init()

        self.log.info("ECE 4760 Laser Projector Controller")
        self.log.info("Starting...")

        Gdk.threads_enter()
        Gtk.main()
        Gdk.threads_leave()


if __name__ == "__main__":
    init_logging(log_level = logging.DEBUG)
    main = MainGUI()
