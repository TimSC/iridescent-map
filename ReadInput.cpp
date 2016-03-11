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

void ReadInput(int zoom, int xtile, int ytile, FeatureStore &featureStore)
{
	// ** Read input file and store in memory using node/way/relation **
	stringstream finaStr;
	finaStr << xtile << "/" << ytile << ".o5m.gz";
	string fina = finaStr.str();
	cout << fina << endl;
	std::filebuf fi;
	fi.open(fina.c_str(), std::ios::in);

	class DecodeGzip fiDec(fi);

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

	featureStore.Clear();
	regrouper.FindAreas(&featureStore);
	regrouper.FindLines(&featureStore);
	regrouper.FindPois(&featureStore);
}

typedef map<int, map<int, OrganisedLabels> > OrganisedLabelsMap;

int main()
{

	// ** Collect labels from off screen tiles
	OrganisedLabelsMap organisedLabelsMap;
	cairo_surface_t *offScreenSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 640, 640);

	for(int x=2034; x <= 2036; x++)
		for(int y=1373; y<= 1375; y++)
		{
			if(x == 2035 && y == 1374) continue;
			FeatureStore featureStore;
			ReadInput(12, x, y, featureStore);
			class SlippyTilesTransform slippyTilesTransform(12, x, y);

			class DrawLibCairoPango drawlib(offScreenSurface);	
			class MapRender mapRender(&drawlib);
			OrganisedLabels organisedLabels;
			
			mapRender.Render(12, featureStore, false, true, slippyTilesTransform, organisedLabels);

			OrganisedLabelsMap::iterator it = organisedLabelsMap.find(x);
			if(it == organisedLabelsMap.end())
				organisedLabelsMap[x] = map<int, OrganisedLabels>();
			map<int, OrganisedLabels> &col = organisedLabelsMap[x];
			col[y] = organisedLabels;
		}

	cairo_surface_destroy(offScreenSurface);

	// ** Render without labels and collect label info **

	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 640, 640);
	FeatureStore featureStore;
	ReadInput(12, 2035, 1374, featureStore);

	class SlippyTilesTransform slippyTilesTransform(12, 2035, 1374);

	class DrawLibCairoPango drawlib(surface);	
	class MapRender mapRender(&drawlib);
	OrganisedLabels organisedLabels;
	
	mapRender.Render(12, featureStore, true, true, slippyTilesTransform, organisedLabels);

	// ** Render labels ** 	

	mapRender.RenderLabels(organisedLabels);

	cairo_surface_write_to_png(surface, "image.png");	
	cairo_surface_destroy(surface);
	return 0;

}

