



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

    

