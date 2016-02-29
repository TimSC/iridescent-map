
#include "Style.h"
#include <iostream>
using namespace std;

int Style::GetStyle(int zoom, const TagMap &tags, FeatureType featuretype, StyleDef &styleDefOut)
{


	TagMap::const_iterator it = tags.find("highway");
	if(it != tags.end())
	{
		StyleAttributes style;
		style["line-color"] = "#ededed";
		LayerDef layerDef;
		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	it = tags.find("landuse");
	if(it != tags.end())
	{
		StyleAttributes style;
		style["fill-color"] = "#444444";
		LayerDef layerDef;
		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	return 0;

}
