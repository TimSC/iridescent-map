#include "cppo5m/OsmData.h"
#include "TagPreprocessor.h"
#include "Regrouper.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "drawlib/drawlibcairo.h"
#include "MapRender.h"
#include "ReadGzip.h"
using namespace std;

void ReadInput(int zoom, int xtile, int ytile, cairo_surface_t *surface)
{
	// ** Read input file and store in memory using node/way/relation **
	stringstream finaStr;
	finaStr << xtile << "/" << ytile << ".o5m.gz";
	string fina = finaStr.str();
	cout << fina << endl;
	std::filebuf fi;
	fi.open(fina.c_str(), std::ios::in);

	class DecodeGzip fiDec(fi);

	class SlippyTilesTransform slippyTilesTransform(zoom, xtile, ytile);

	class Regrouper regrouper;

	class TagPreprocessor tagPreprocessor;
	tagPreprocessor.output = &regrouper;	

	class O5mDecode dec(fiDec);
	dec.output = &tagPreprocessor;
	dec.DecodeHeader();

	while (fiDec.in_avail()>0)
		dec.DecodeNext();
	
	//Iterates over object lists and regenerates internal ID indexing map structures.
	regrouper.UpdateIdMappings();

	// ** Change to area/line/POI representation **

	FeatureStore featureStore;
	regrouper.FindAreas(&featureStore);
	regrouper.FindLines(&featureStore);
	regrouper.FindPois(&featureStore);

	// ** Render without labels and collect label info **

	class DrawLibCairoPango drawlib(surface);	
	class MapRender mapRender(&drawlib);
	OrganisedLabels organisedLabels;
	
	mapRender.Render(zoom, featureStore, slippyTilesTransform, organisedLabels);
	mapRender.RenderLabels(organisedLabels);
}

int main()
{
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 640, 640);

	ReadInput(12, 2035, 1374, surface);

	cairo_surface_write_to_png(surface, "image.png");	
	cairo_surface_destroy(surface);
	return 0;

}

