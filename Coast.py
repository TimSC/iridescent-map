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

	print coast.GetVal(2035, 1373), True
	print coast.GetVal(2037, 1376), False
	print coast.GetVal(2225, 1626), False
	print coast.GetVal(1985, 1611), True
	print coast.GetVal(1008, 1769), False
	print coast.GetVal(637, 1485), True
	print coast.GetVal(632, 1485), False
	print coast.GetVal(4049, 2523), True

	#for x in range(coast.width):
	#	for y in range(coast.height):
	#		print x, y, coast.GetVal(x, y)

