import struct

class CoastMap(object):
	def __init__(self, fi, zoom):
		self.fi = fi
		self.fi.seek(0)
		
		sizeValsRaw = self.fi.read(8)
		self.width, self.height = struct.unpack("!ii", sizeValsRaw)
		self.zoom = zoom

	def GetVal(self, x, y):
		if x >= self.width or y >= self.height:
			raise RuntimeError("Out of bounds")
		targetPos = 8 + (y * self.width / 8) + x / 8
		self.fi.seek(targetPos)
		byte = struct.unpack("<B", self.fi.read(1))[0]
		return (byte & (1 << (7-(x % 8)))) != 0

if __name__=="__main__":
	coast = CoastMap(open("../iridescent-testdata/map.bin"), 12)

	print coast.GetVal(2036, 1376)
	#for x in range(coast.width):
	#	for y in range(coast.height):
	#		print x, y, coast.GetVal(x, y)

