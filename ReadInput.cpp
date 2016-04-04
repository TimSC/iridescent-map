///\file
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

/** \mainpage

Renderer API

                          (CppSharp)  native (Python)
                              |---------|-------|    
                                        |             

The ReadInput() function loads a data source into a POI, line, area representation, which is a higher level preprocessed representation than the usual node, way, relation representation.

- DecodeGzip decodes the binary file into a o5m stream
- O5mDecode decodes this stream and passes it to TagPreprocessor
- TagPreprocessor does basic tag filtering and passes the data to Regrouper
- Regrouper changes ways and relations to lines and areas.
- The resulting data is returned from ReadInput in a FeatureStore object

The rest of the functionality is in main() :

MapRender takes FeatureStore data, a transform object derived from ITransform (in this case a SlippyTilesTransform) and a drawing surface object derived from IDrawLib (in this case DrawLibCairoPango). During MapRender::Render(), it (optionally) draws map shapes on to the drawing surface and (optionally) outputs an LabelsByImportance object.

The labels from surrounding tiles are then combined into a RenderLabelList and passed to MapRender::RenderLabels() for drawing to the IDrawLib drawing surface. This is responsible for merging equivalent labels, checking the label actually fits in the allowed space, ensuring it is the right way up, etc.

\section oldplan Old plan for design

This is still vaguely correct.

                              Data source    Source definition
                                     |              |
                             Input selector --------|
                                     |
                              Tag preprocessor -- Preprocessor def
                                          |
                                     Regroup objects -- Style def
                                          |
	                                 Layer draw manager
                                       |          |
                                       |          |
    Style parser --Style def -- Way processor      Area/POI processor---- Style def
                                       |           |
									Map transform--|
                                       |           |
                                       |-----------|----Icon/Label engine
                                       |           |
    Render engine                     Renderer ----|
                                        |
                          |-------------|-----------|----------|--------|
    Backends            Pango      (OpenGL ES)    Cairo     (WebGL)   (SVG)

See also: http://wiki.osgeo.org/wiki/OSGeo_Cartographic_Engine_Discussion

Show labels based on their relative importance and up to a clutter limit (rather than static fixed approach?)

*/

///ReadInput reads a file, preprocesses it and outputs a FeatureStore representation of the data.
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

typedef map<int, map<int, LabelsByImportance> > OrganisedLabelsMap;

///Main function for program
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
			LabelsByImportance organisedLabels;
			
			mapRender.Render(12, featureStore, false, true, slippyTilesTransform, organisedLabels);

			OrganisedLabelsMap::iterator it = organisedLabelsMap.find(x);
			if(it == organisedLabelsMap.end())
				organisedLabelsMap[x] = map<int, LabelsByImportance>();
			map<int, LabelsByImportance> &col = organisedLabelsMap[x];
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
	LabelsByImportance organisedLabels;
	
	mapRender.Render(12, featureStore, true, true, slippyTilesTransform, organisedLabels);
	organisedLabelsMap[2035][1374] = organisedLabels;

	// ** Render labels **
	RenderLabelList labelList;
	RenderLabelListOffsets labelOffsets;
	for(int y=1373; y<= 1375; y++)
	{
		for(int x=2034; x <= 2036; x++)
		{
			map<int, LabelsByImportance> &col = organisedLabelsMap[x];
			labelList.push_back(col[y]);
			labelOffsets.push_back(std::pair<double, double>(640.0*(x-2035), 640.0*(y-1374)));
		}
	}

	mapRender.RenderLabels(labelList, labelOffsets);

	cairo_surface_write_to_png(surface, "image.png");	
	cairo_surface_destroy(surface);
	return 0;

}

