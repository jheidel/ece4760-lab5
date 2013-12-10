



class IldaFrame:

    def __init__(self, points):
        # List of ((x,y,z), blanking) tuples
        self.points = points
        self.length = len(points)

    def get_length(self):
        return self.length

    def get_points(self):
        pts = {}
        for ((x,y,z),b) in self.points:
            pts["x"] = x
            pts["y"] = y
            pts["z"] = z
            pts["blank"] = b
            yield pts

    def get_mapped_points(self):
        pts = {}
        def map_pt(p):
            #return ((p + 2**15) / 16) / 8 + 2048
            return ((p + 2**15) / 16)
        for ((x,y,z),b) in self.points:
            pts["x"] = map_pt(x)
            pts["y"] = map_pt(y)
            pts["blank"] = b
            yield pts


    @classmethod
    def SqWaveTestPattern(cls, x=True):
        pts = []
        max = 2**15-100
        stepsize = max * 2 / 500
        sq1 = -max/2
        sq2 = max/2
        
        #Generate square wave
        for i in range(-max, max, stepsize):
            if x:
                pts.append(((i, -max if (i < sq1 or i > sq2) else max, 0), False))
            else:
                pts.append(((-max if (i < sq1 or i > sq2) else max, i, 0), False))

        #Generate blank return pass to origin
        for i in range(max, -max, -stepsize * 2):
            if x:
                pts.append(((i, -max, 0), True))
            else:
                pts.append(((-max, i, 0), True))
        pts.append(((-max, -max, 0), True))

        return cls(pts)
