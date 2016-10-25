import struct, math
from PIL import Image

class CoastMap(object):
	def __init__(self, fi):
		self.fi = fi
		self.fi.seek(0)
		
		sizeValsRaw = self.fi.read(12)
		self.width, self.height, self.zoom = struct.unpack("!LLL", sizeValsRaw)
		print self.width, self.height, self.zoom

	def GetVal(self, y, x):
		if x >= self.width or y >= self.height:
			raise RuntimeError("Out of bounds")
		targetPos = 12 + (y * math.ceil(self.width / 8.0)) + (x / 8)
		self.fi.seek(targetPos)
		byte = struct.unpack("B", self.fi.read(1))[0]
		return (byte & (1 << (7-(x % 8)))) != 0, byte

class CoastTxt(object):
	def __init__(self, fi):
		self.fi = fi
		self.fi.seek(0)
		self.width = int(self.fi.readline().strip())
		self.height = int(self.fi.readline().strip())
		self.zoom = int(self.fi.readline().strip())
		self.datStart = self.fi.tell()	

	def GetVal(self, y, x):
		targetPos = self.datStart + (x/8)*8 + y * self.width 
		self.fi.seek(targetPos)
		dat = self.fi.read(8)
		return dat[x%8] == "1", dat

if __name__=="__main__":
	coast = CoastMap(open("../iridescent-testdata/map.bin"))
	im = Image.open("../iridescent-testdata/fosm-coast-earth20160927080659.png")
	img = im.load()
	coastTxt = CoastTxt(open("../iridescent-testdata/map.txt"))

	print coast.GetVal(2035, 1373), img[2035, 1373]>0, coastTxt.GetVal(2035, 1373), True
	print coast.GetVal(2037, 1376), img[2037, 1376]>0, coastTxt.GetVal(2037, 1376), False
	print coast.GetVal(2225, 1626), img[2225, 1626]>0, coastTxt.GetVal(2225, 1626), False
	print coast.GetVal(1985, 1611), img[1985, 1611]>0, coastTxt.GetVal(1985, 1611), True
	print coast.GetVal(1008, 1769), img[1008, 1769]>0, coastTxt.GetVal(1008, 1769), False
	print coast.GetVal(637, 1485), img[637, 1485]>0, coastTxt.GetVal(637, 1485), True
	print coast.GetVal(632, 1485), img[632, 1485]>0, coastTxt.GetVal(632, 1485), False
	print coast.GetVal(4049, 2523), img[4049, 2523]>0, coastTxt.GetVal(4049, 2523), True

	#for x in range(coast.width):
	#	for y in range(coast.height):
	#		print x, y, coast.GetVal(x, y)

