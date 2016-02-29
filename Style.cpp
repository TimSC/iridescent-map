
#include "Style.h"
#include <iostream>
using namespace std;

int Style::GetStyle(int zoom, const TagMap &tags, FeatureType featuretype, StyleDef &styleDefOut)
{


	TagMap::const_iterator it = tags.find("highway");
	if(it != tags.end() && featuretype == Line)
	{
		StyleAttributes style;
		style["line-color"] = "#000000";
		style["line-width"] = "3";

		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(3);

		StyleAttributes style2;
		style2["line-color"] = "#ffffff";
		style2["line-width"] = "2";
		LayerDef layerDef2;
		layerDef.push_back(1);
		layerDef.push_back(4);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		styleDefOut.push_back(StyleAndLayerDef(layerDef2, style2));
		return 1;
	}

	it = tags.find("landuse");
	if(it != tags.end() && featuretype == Area)
	{
		StyleAttributes style;
		style["fill-color"] = "#44aa44";
		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(1);
		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	return 0;

}
