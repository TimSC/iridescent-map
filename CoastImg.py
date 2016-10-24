
from PIL import Image

if __name__=="__main__":
	im = Image.open("../iridescent-testdata/fosm-coast-earth20160927080659.png")
	img = im.load()

	print img[2035, 1373], True
	print img[2037, 1376], False
	print img[2225, 1626], False
	print img[1985, 1611], True
	print img[1008, 1769], False
	print img[637, 1485], True
	print img[632, 1485], False
	print img[4049, 2523], True


