#include "cppo5m/OsmData.h"
#include "TagPreprocessor.h"
#include "Regrouper.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "drawlib/drawlibcairo.h"
#include "MapRender.h"
using namespace std;

void ReadInput(int zoom, int xtile, int ytile, cairo_surface_t *surface)
{
	stringstream finaStr;
	finaStr << xtile << "/" << ytile << ".o5m.gz";
	string fina = finaStr.str();
	cout << fina << endl;
	std::ifstream fi(fina.c_str());

	class SlippyTilesTransform slippyTilesTransform(zoom, xtile, ytile);

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
	
	class DrawLibCairoPango drawlib(surface);
	
	class MapRender mapRender(&drawlib);
	
	mapRender.Render(zoom, featureStore, slippyTilesTransform);
}

int main()
{
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 640, 640);

	ReadInput(12, 2035, 1374, surface);

	cairo_surface_write_to_png(surface, "image.png");	
	cairo_surface_destroy(surface);
	return 0;

}

