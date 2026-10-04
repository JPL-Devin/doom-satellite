"""DoomFrameReader.py: Python implementation of Components.DoomFrameReader (fprime-python)

Reads the downsampled DOOM frames repeated by Components.FrameRepeater and reports frame statistics as telemetry.
The handlers run on the F Prime thread that invokes the port (the DOOM rate group), holding the interpreter lock, so
the per-frame work is kept to C-speed operations on the pixel memoryview.
"""
from DoomFrameReaderBaseAc import DoomFrameReaderBase

# BT.601 luma weights, used to turn each palette entry into a 0-255 brightness
LUMA_WEIGHTS = (0.299, 0.587, 0.114)


class DoomFrameReader(DoomFrameReaderBase):
    """Reads frames and palettes; see DoomFrameReader.fpp for the interface"""

    def __init__(self):
        super().__init__()
        self.frames_read = 0
        self.palettes_read = 0
        # 256-byte translation table: palette index -> brightness, None until a palette arrives
        self.brightness_table = None

    def frameIn_handler(self, portNum, frameNumber, width, height, pixels):
        """Read one frame: width * height palette indices in the caller's buffer, valid only during this call"""
        count = width * height
        size = pixels.getSize()
        if count == 0 or size < count:
            self.log_WARNING_LO_FrameRejected(width, height, size)
            return
        indices = pixels.getData()[:count]
        self.frames_read += 1
        self.tlmWrite_FramesRead(self.frames_read)
        self.tlmWrite_LastFrame(frameNumber)
        self.tlmWrite_MeanPixel(sum(indices) / count)
        if self.brightness_table is not None:
            brightness = indices.tobytes().translate(self.brightness_table)
            self.tlmWrite_MeanBrightness(sum(brightness) / count)

    def paletteIn_handler(self, portNum, palette):
        """Rebuild the brightness table from the 256 RGB triples of the palette"""
        rgb = palette.get_rgb()
        table = bytearray(256)
        for entry in range(256):
            table[entry] = min(
                255,
                int(sum(weight * rgb[3 * entry + channel] for channel, weight in enumerate(LUMA_WEIGHTS))),
            )
        self.brightness_table = bytes(table)
        self.palettes_read += 1
        self.tlmWrite_PalettesRead(self.palettes_read)
