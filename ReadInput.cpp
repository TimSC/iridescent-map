#include "cppo5m/OsmData.h"
#include "TagPreprocessor.h"
#include "Regrouper.h"
#include <fstream>
#include <iostream>
#include "drawlib/drawlibcairo.h"
#include "MapRender.h"
using namespace std;

int main()
{
	std::ifstream fi("1374.o5m");

	class SlippyTilesTransform slippyTilesTransform(12, 2035, 1374);

	class Regrouper regrouper;

	class TagPreprocessor tagPreprocessor;
	tagPreprocessor.output = &regrouper;	

	class O5mDecode dec(fi);
	dec.output = &tagPreprocessor;
	dec.DecodeHeader();

	while (!fi.eof())
		dec.DecodeNext();
	
	regrouper.UpdateIdMappings();

	FeatureStore featureStore;
	regrouper.FindAreas(&featureStore);
	regrouper.FindLines(&featureStore);
	regrouper.FindPois(&featureStore);
	
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 640, 640);
	class DrawLibCairoPango drawlib(surface);
	
	class MapRender mapRender(&drawlib);
	
	mapRender.Render(12, featureStore, slippyTilesTransform);

	cairo_surface_write_to_png(surface, "image.png");	
	cairo_surface_destroy(surface);
	return 0;

}

