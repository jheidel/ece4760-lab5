from threading import Thread, Event, Condition
from multiprocessing import Process
from ctypes import *
from pylib.log import with_logging
from time import sleep, time

try:
    clib = CDLL("clib/clib.so")
except:
    print "C shared library missing!"
    print "Did you forget to \"make\"?"
    sys.exit(1)

@with_logging
class SerialComm(Thread):
    def __init__(self, port):
        Thread.__init__(self)
        self.log.info("Starting serial communications on port %s" % port)
        ret = clib.init_serial(port)
        if ret != 0:
            raise Exception("SERIAL LIBRARY RETURNED %d" % ret)

        clib.scanner_init()

        self.kill = Event()
        self.c = Condition()
        self.frame = None
        self.set_pps(800) #default PPS

    def set_point(self, x, y, blank):
        """
        Set point for the laser (x,y) with blank indicating whether the
        laser is blanked or not
        """
        #self.log.info("Setting point %d, %d" % (x,y))
        ret = clib.serial_new_point(x, y, blank)
        if ret != 0:
            raise Exception("SERIAL LIBRARY RETURNED %d" % ret)


    #Conversion from ILDA point space to laser coordinates
    def set_ilda_point(self, x, y, blank):
        def map_pt(p):
            return (p + 2**15) / 16
        self.set_point(map_pt(x), map_pt(y), blank)

    def stop(self):
        clib.scanner_stop()
        sleep(2 * 1.0 / self.pps)
        self.set_point(0,0,True)
        """
        with self.c:
            self.frame = None
            self.kill.set()
            self.c.notify()
        """

    def set_frame(self, frame):
        """
        Sets a new ilda frame for the laser projector to scan through
        None indicates that scanning should stop and the laser should be blanked
        """

        if frame is not None:
            if frame != self.frame:
                self.frame = frame
                #self.log.debug("Frame has %d points" % frame.get_length()) 
                clib.new_point_set(frame.get_length())
                for i,pt in enumerate(frame.get_mapped_points()):
                    clib.set_point_by_index(i, pt["x"], pt["y"], pt["blank"]) 
                clib.activate_point_set()
        else:
            self.set_point(0,0,True)

        """
        with self.c:
            self.frame = frame
            self.frame_changed = True
            if frame is not None:
                self.log.debug("Frame has %d points" % len(frame.points))
            self.c.notify()
        """

    def set_pps(self, pps):
        """
        Sets a new scanning speed for the ilda frame
        """
        self.pps = pps
        self.log.info("Settings laser to %d PPS" % pps)
        clib.scanner_set_pps(pps)
        """
        with self.c:
            self.pps = pps
            self.frame_changed = True
        """

    def run(self):
        #ret = clib.scanner_main()
        #if ret != 0:
        #    raise Exception("SCANNER LIBRARY MAIN RETURNED %d" % ret)
        pass
        """
        while True:
            with self.c:
                if self.frame is None:
                    self.set_point(0, 0, True)
                    self.c.wait()
                    continue
                pts = self.frame.get_points()
                pps = self.pps

            t = time() + 1.0 / self.pps
            for pt in pts:

                self.set_ilda_point(pt["x"], pt["y"], pt["blank"])
                #if t > time():
                #    self.log.warning("TIME OVERFLOW")
                #t += 1.0 / pps

                if self.kill.is_set():
                    return

                if self.frame_changed:
                    self.frame_changed = False
                    break #load new frame
        """
                    
                        

