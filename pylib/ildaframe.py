



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
            return (p + 2**15) / 16
        for ((x,y,z),b) in self.points:
            pts["x"] = map_pt(x)
            pts["y"] = map_pt(y)
            pts["blank"] = b
            yield pts


    

