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
def getStringFromMethod(method):
    method.argtypes = ()
    method.restype = POINTER(c_char_p)
    return cast(method(), c_char_p).value

class IldaParser:

    def __init__(self, filename):
        """
        Loads ILDA file and stores frames
        """
        self.frames = []
        clib.loadILDAFile(filename)
        first = True

        while True:
            clib.readHeader()

            if first:
                first = False
                self.name = getStringFromMethod(clib.getName)
                self.companyname = getStringFromMethod(clib.getCompanyName)

            if (not clib.isDataFrame()):
                #print "Skipping non-data frame"
                continue
            
            # Parse Header
            points = []

            for i in range(clib.getEntries()):
                clib.loadNextPoint()
                points.append( ((pointData[0], #x
                                pointData[1], #y
                                pointData[2]), #z
                                laserData[0] == 1)) #blanking bit



            self.frames.append(IldaFrame(points))

            if (clib.getFrameNum() >= clib.getTotalFrames() - 1):
                break #Finished parsing


    def getFrames(self):
        return self.frames

    def getTitle(self):
        return self.name + self.companyname

