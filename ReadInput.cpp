///\file
#include "TagPreprocessor.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "cppGzip/DecodeGzip.h"
#include "ReadInput.h"

using namespace std;

/** \mainpage

The ReadInput() function loads a data source into a POI, line, area representation, which is a higher level preprocessed representation than the usual node, way, relation representation.

- DecodeGzip decodes the binary file into a o5m stream
- O5mDecode decodes this stream and passes it to TagPreprocessor
- TagPreprocessor does basic tag filtering and passes the data to Regrouper
- Regrouper changes ways and relations to lines and areas.
- The resulting data is returned from ReadInput in a FeatureStore object

The rest of the functionality is in main() :

MapRender takes FeatureStore data, a transform object derived from ITransform (in this case a SlippyTilesTransform) and a drawing surface object derived from IDrawLib (in this case DrawLibCairoPango). During MapRender::Render(), it (optionally) draws map shapes on to the drawing surface and (optionally) outputs an LabelsByImportance object.

The labels from surrounding tiles are then combined into a RenderLabelList and passed to MapRender::RenderLabels() for drawing to the IDrawLib drawing surface. This is responsible for merging equivalent labels, checking the label actually fits in the allowed space, ensuring it is the right way up, etc.

\section oldplan Overall design concept

                              Data source    Source definition
                                     |              |
                             Input selector --------|
                                     |
                                TagPreprocessor
                                        |
                                     Regrouper
                                        |
	                                  MapRender
                                        |
                                  IDrawLib based renderer
                                        |
                          |-------------|-----------|----------|--------|
    Backends            Pango      (OpenGL ES)    Cairo     (WebGL)   (SVG)

\subsection api Render API

                          (CppSharp)  native (Python)
                              |---------|-------|    
                                        |             

\subsection maprender MapRender

                                     FeatureConverter ---- Style
                                       |          |    |
                                       |          |    --- ITransform
                                       |          |
                          FeaturesToDrawCmds   FeaturesToLabelEngine
                                       |          |
									drawTree   FeaturesToLabelEngine::poiLabels
                                       |          |
             DrawTreeNode::WriteDrawCommands   LabelEngine
                                       |          |
                                       ▼          ▼
                                          output


See also: http://wiki.osgeo.org/wiki/OSGeo_Cartographic_Engine_Discussion

Show labels based on their relative importance and up to a clutter limit (rather than static fixed approach?)

*/

///ReadInput reads a file, preprocesses it and outputs a FeatureStore representation of the data.
void ReadInput(int zoom, const char *basePath, int xtile, int ytile, FeatureStore &featureStore)
{
	// ** Read input file and store in memory using node/way/relation **
	stringstream finaStr;
	finaStr << basePath << "/";
	finaStr << zoom << "/";
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

