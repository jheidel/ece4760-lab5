from gi.repository import Gtk, Gdk, Pango, PangoCairo
import cairo
import math

class LaserViz(Gtk.DrawingArea):

    def __init__(self, parent):
        Gtk.DrawingArea.__init__(self)
        self.connect('draw', self._do_expose)
        self.parent = parent
        self.show()
        self.container = parent.builder.get_object("laserVizBox")
        self.container.pack_start(self, True, True, 0)

        self.ildaframe = None

    def set_frame(self, frame):
        self.ildaframe = frame


    def _do_expose(self, widget, cr):
        allocation = self.get_allocation()
        width = allocation.width
        height = allocation.height

        #draw background
        cr.set_source_rgb(0,0,0)
        cr.rectangle(0,0, width, height)
        cr.fill()

        if self.ildaframe is not None:
            print "DRAW"
            cr.set_line_width(1.0)
            past_xy = None
            for pt in self.ildaframe.get_points():
                
                if (pt["blank"]):
                    cr.set_source_rgb(0,0.2,0) #green laser!
                else:
                    cr.set_source_rgb(0,1.0,0) #green laser!


                draw_x = width / 2 + float(pt["x"]) / 2**16 * width
                draw_y = height / 2 - float(pt["y"]) / 2**16 * height
                cr.arc(draw_x, draw_y, 1, 0, 2 * math.pi)
                cr.fill()

                if past_xy is not None:
                    #Draw line from past to present
                    (px, py) = past_xy
                    cr.move_to(px, py)
                    cr.line_to(draw_x, draw_y)
                    cr.stroke()
                past_xy = (draw_x, draw_y)







