
#include "Style.h"
#include <iostream>
using namespace std;

int Style::GetStyle(int zoom, const TagMap &tags, FeatureType featuretype, StyleDef &styleDefOut)
{
	TagMap::const_iterator it = tags.find("highway");
	if(it != tags.end() && featuretype == Line)
	{
		StyleAttributes style, style2;
		LayerDef layerDef, layerDef2;

		bool knownType = false;
		if(it->second == "motorway")
		{
			knownType = true;
			style["line-color"] = "#000055";
			style["line-width"] = "8";
			layerDef.push_back(1);
			layerDef.push_back(4);

			style2["line-color"] = "#0000aa";
			style2["line-width"] = "6";
			layerDef2.push_back(1);
			layerDef2.push_back(10);
		}

		if(it->second == "trunk")
		{
			knownType = true;
			style["line-color"] = "#005500";
			style["line-width"] = "7";
			layerDef.push_back(1);
			layerDef.push_back(3);

			style2["line-color"] = "#00aa00";
			style2["line-width"] = "5";
			layerDef2.push_back(1);
			layerDef2.push_back(9);
		}

		if(it->second == "primary")
		{
			knownType = true;
			style["line-color"] = "#550000";
			style["line-width"] = "5";
			layerDef.push_back(1);
			layerDef.push_back(2);

			style2["line-color"] = "#aa0000";
			style2["line-width"] = "3";
			layerDef2.push_back(1);
			layerDef2.push_back(8);
		}

		if(it->second == "secondary")
		{
			knownType = true;
			style["line-color"] = "#55555500";
			style["line-width"] = "4";
			layerDef.push_back(1);
			layerDef.push_back(1);

			style2["line-color"] = "#aaaaaa00";
			style2["line-width"] = "3";
			layerDef2.push_back(1);
			layerDef2.push_back(7);
		}
		
		if(!knownType)
		{
			style["line-color"] = "#000000";
			style["line-width"] = "3";
			layerDef.push_back(1);
			layerDef.push_back(0);

			style2["line-color"] = "#aaaaaa";
			style2["line-width"] = "2";
			layerDef2.push_back(1);
			layerDef2.push_back(6);
		}

		style["line-join"] = "round";
		style["line-cap"] = "round";
		style2["line-join"] = "round";
		style2["line-cap"] = "round";

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		styleDefOut.push_back(StyleAndLayerDef(layerDef2, style2));

		return 1;
	}

	it = tags.find("landuse");
	if(it != tags.end() && featuretype == Area)
	{
		StyleAttributes style;
		style["polygon-fill"] = "#444444";
		style["polygon-gamma"] = "1.0";

		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(1);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	it = tags.find("natural");
	if(it != tags.end() && it->second == "water" && featuretype == Area)
	{
		StyleAttributes style;
		style["polygon-fill"] = "#0000ff";

		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(2);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}
	if(it != tags.end() && it->second == "wood" && featuretype == Area)
	{
		StyleAttributes style;
		style["polygon-fill"] = "#00aa00";

		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(2);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	it = tags.find("building");
	if(it != tags.end() && featuretype == Area)
	{
		StyleAttributes style;
		style["polygon-fill"] = "#aa7777";

		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(3);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	it = tags.find("amenity");
	if(it != tags.end() && featuretype == Poi && it->second == "pub")
	{
		StyleAttributes style;
		style["text-name"] = "[name]";
		style["text-size"] = "9";
		style["marker-file"] = "url('symbols/pub.16.svg')";
		style["marker-fill"] = "#734a08";

		LayerDef layerDef;
		layerDef.push_back(2);
		layerDef.push_back(1);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	return 0;

}
