import sys
from gi.repository import Gtk, Gdk, GLib

from pylib.ildaparser import IldaParser
from pylib.laserviz import LaserViz


p = IldaParser(sys.argv[1])


from threading import Thread, Event
from time import sleep

class FramePlayer(Thread):
    def __init__(self, lv, p):
        Thread.__init__(self)
        self.lv = lv
        self.p = p
        self.kill = Event()
        self.FPS = 15
        self.start()

    def stop(self):
        self.kill.set()

    def run(self):
        while True:
            for f in self.p.get_frames():
                self.lv.set_frame(f)
                self.kill.wait(1.0/self.FPS)
                if (self.kill.is_set()):
                    return


class MainGUI:

    def window_destroy(self, event):
        Gtk.main_quit()
        self.player.stop()

    def __init__(self):
        self.gladefile = "pylib/gui.glade"

        self.builder = Gtk.Builder()
        self.builder.add_from_file(self.gladefile)
        self.builder.connect_signals(self)

        self.laserviz = LaserViz(self)

        self.window = self.builder.get_object("mainWindow")
        self.window.show()

        #self.laserviz.set_frame(p.frames[0])
        self.player = FramePlayer(self.laserviz, p)
        

        Gdk.threads_init()
        GLib.threads_init()

        Gdk.threads_enter()
        Gtk.main()
        Gdk.threads_leave()


if __name__ == "__main__":
    main = MainGUI()
