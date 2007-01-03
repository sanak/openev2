
class GvBitLayerLUT:
    """Class for managing a LUT for display bitplane rasters.

    This class keeps state for managing 8 colourized bitplanes, and
    can produce an appropriate lut in string format, suitable for using
    with GvRasterLayer.put_lut() when needed."""

    def __init__(self):
        """Initialize with eight distinct colours for bit planes."""
        self.plane_color = [(0,0,0,0), (255,0,0,255), (0,255,0,255), 
                            (0,0,255,255), (255,255,0,255), (255,0,255,255), 
                            (0,255,255,255), (255,255,255,255)]
        self.priority = [0, 1, 2, 3, 4, 5, 6, 7]

    def set_color(self,plane,color):
        """Set the color of one of the bit planes.

        plane -- a bit plane number from 0 to 7.

        color -- an RGBA tuple such as (255,0,0,255)"""
        self.plane_color[plane] = color

    def set_priority(self,priority,plane):
        """Set what plane is drawn at a given priority level.

        By default (after initialization) bit plane zero is
        priority zero, through to bit plane seven being at
        priority seven.  To move bit plane 0 to top priority, and
        move everything else down one it would be necessary to do
        something like this example:

        bit_lut.set_priority(7,0)
        bit_lut.set_priority(6,7)
        bit_lut.set_priority(5,6)
        bit_lut.set_priority(4,5)
        bit_lut.set_priority(3,4)
        bit_lut.set_priority(2,3)
        bit_lut.set_priority(1,2)
        bit_lut.set_priority(0,1)

        priority -- a priority level from 0 to 7

        plane -- a plane from 0 to 7"""

        self.priority[priority] = plane

    def get_lut_as_string(self):
        """Fetch a lut suitable for use with GvRasterLayer.lut_put()
        representing the current bit plane configuration."""
        lut = []
        for i in range(256):
            lut.append((0,0,0,0))

        for ip in range(8):
            plane = self.priority[ip]
            bitvalue = 2 ** plane
            for i in range(256):
                if (i % (bitvalue*2)) >= bitvalue:
                    lut[i] = self.plane_color[plane]

        lut_string = ''
        for i in range(256):
            rgba_tuple = lut[i]
            lut_string = lut_string + \
                (chr(rgba_tuple[0])+chr(rgba_tuple[1])+ \
                 chr(rgba_tuple[2])+chr(rgba_tuple[3]))
        return lut_string

if __name__ == '__main__':
    x = GvBitLayerLUT()

    print x.get_lut_as_string()
