from ctypes import *
from ildaframe import IldaFrame
import sys

try:
    clib = CDLL("clib/clib.so")
except:
    print "C shared library missing!"
    print "Did you forget to \"make\"?"
    sys.exit(1)

getPointData = clib.getPointData
getPointData.argtypes = ()
getPointData.restype = POINTER(c_int16)

pointData = getPointData() #by reference!

getLaserData = clib.getLaserData
getLaserData.argtypes = ()
getLaserData.restype = POINTER(c_uint8)

laserData = getLaserData() #by reference!

class IldaParseException(Exception):
    def __init__(self, message):
        Exception.__init__(self, message)


#helper for converting ctype strings
def getStringFromMethod(method, arg):
    method.argtypes = ()
    method.restype = POINTER(c_char_p)
    return cast(method(arg), c_char_p).value

class IldaParser:

    def __init__(self, filename):
        """
        Loads ILDA file and stores frames
        """
        self.frames = []
        clib.loadILDAFile(filename)


        self.fileobject = clib.getIldaFilePointer()
        self.doneparse = False


    def __del__(self):
        #TODO: verify
        try:
            clib.closeFile(self.fileobject)
        except:
            pass

    def get_initial_frame(self):
        for f in self.get_frames():
            return f
        return None

    ## Generator to get frames from file
    def get_frames(self):
        i = 0
        first = True

        while True:
            if self.doneparse or i < len(self.frames):
                #print "Read frame %d from cache" % i
                if self.doneparse and i >= len(self.frames):
                    return
                i += 1
                yield self.frames[i-1]
            else:

                #print "Read frame %d from file" % i
                clib.readHeader(self.fileobject)

                if first:
                    first = False
                    self.name = getStringFromMethod(clib.getName, self.fileobject)
                    self.companyname = getStringFromMethod(clib.getCompanyName, self.fileobject)

                if (clib.getFormatType(self.fileobject) > 3):
                    raise Exception("Unknown ilda format type: %d" % clib.getFormatType(self.fileobject))

                if (not clib.isDataFrame(self.fileobject)):
                    #print "Skipping non-data frame"
                    continue
                
                # Parse Header
                points = []

                for j in range(clib.getEntries(self.fileobject)):
                    clib.loadNextPoint(self.fileobject)
                    points.append( ((pointData[0], #x
                                    pointData[1], #y
                                    pointData[2]), #z
                                    laserData[0] == 1)) #blanking bit


                self.frames.append(IldaFrame(points))
                i += 1

                if (clib.getFrameNum(self.fileobject) >= clib.getTotalFrames(self.fileobject) - 1):
                    self.doneparse = True
                    #return #finished parsing

                yield self.frames[i-1]


    def getTitle(self):
        return self.name + self.companyname

