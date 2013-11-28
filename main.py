import sys
from gi.repository import Gtk, Gdk, GLib

from pylib.ildaparser import IldaParser
from pylib.laserviz import LaserViz


p = IldaParser(sys.argv[1])

class MainGUI:

    def window_destroy(self, event):
        Gtk.main_quit()

    def __init__(self):
        self.gladefile = "pylib/gui.glade"

        self.builder = Gtk.Builder()
        self.builder.add_from_file(self.gladefile)
        self.builder.connect_signals(self)

        self.laserviz = LaserViz(self)

        self.window = self.builder.get_object("mainWindow")
        self.window.show()

        self.laserviz.set_frame(p.frames[0])

        Gdk.threads_init()
        GLib.threads_init()

        Gdk.threads_enter()
        Gtk.main()
        Gdk.threads_leave()


if __name__ == "__main__":
    main = MainGUI()
