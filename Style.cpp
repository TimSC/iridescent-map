
#include "Style.h"
#include <iostream>
#include <cstring>
#include <stdio.h>
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
			style["line-color"] = "#7d92ae";
			style["line-width"] = "8";
			layerDef.push_back(1);
			layerDef.push_back(4);

			style2["line-color"] = "#89a4ca";
			style2["line-width"] = "6";
			layerDef2.push_back(1);
			layerDef2.push_back(10);
		}

		if(it->second == "trunk")
		{
			knownType = true;
			style["line-color"] = "#87b988";
			style["line-width"] = "7";
			layerDef.push_back(1);
			layerDef.push_back(3);

			style2["line-color"] = "#87b988";
			style2["line-width"] = "5";
			layerDef2.push_back(1);
			layerDef2.push_back(9);
		}

		if(it->second == "primary")
		{
			knownType = true;
			style["line-color"] = "#dc9e9e";
			style["line-width"] = "5";
			layerDef.push_back(1);
			layerDef.push_back(2);

			style2["line-color"] = "#dc9e9e";
			style2["line-width"] = "3";
			layerDef2.push_back(1);
			layerDef2.push_back(8);
		}

		if(it->second == "secondary")
		{
			knownType = true;
			style["line-color"] = "#d4ae7b";
			style["line-width"] = "4";
			layerDef.push_back(1);
			layerDef.push_back(1);

			style2["line-color"] = "#f8d5a9";
			style2["line-width"] = "3";
			layerDef2.push_back(1);
			layerDef2.push_back(7);
		}
		
		if(!knownType)
		{
			style["text-name"] = "[name]";
			style["text-size"] = "9";
			style["text-importance"] = "1";
			style["text-placement"] = "line";
			style["line-color"] = "#cdcdcc";
			style["line-width"] = "4";
			layerDef.push_back(1);
			layerDef.push_back(0);

			style2["line-color"] = "#fefefe";
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
		style["polygon-fill"] = "#dcdcdc";
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
		style["polygon-fill"] = "#b5d0d0";
		style["text-name"] = "[name]";
		style["text-fill"] = "#7aa6d2";
		style["text-size"] = "9";
		style["text-importance"] = "2";
		style["text-halo-fill"] = "#ffffff";

		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(2);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}
	if(it != tags.end() && it->second == "wood" && featuretype == Area)
	{
		StyleAttributes style;
		style["polygon-fill"] = "#aed1a0";

		LayerDef layerDef;
		layerDef.push_back(1);
		layerDef.push_back(2);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}
	if(it != tags.end() && it->second == "coastline")
	{
		StyleAttributes style;
		style["coastline"] = "yes";

		LayerDef layerDef;

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	it = tags.find("building");
	if(it != tags.end() && featuretype == Area)
	{
		StyleAttributes style;
		style["polygon-fill"] = "#beacac";

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
		//style["text-name"] = "[name]";
		//style["text-size"] = "9";
		//style["text-importance"] = "0";
		//style["text-halo-fill"] = "#0008";
		style["marker-file"] = "url('symbols/pub.16.svg')";
		style["marker-fill"] = "#734a08";

		LayerDef layerDef;
		layerDef.push_back(2);
		layerDef.push_back(1);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	it = tags.find("place");
	if(it != tags.end() && featuretype == Poi && it->second == "suburb")
	{
		StyleAttributes style;
		style["text-name"] = "[name]";
		style["text-size"] = "12";
		style["text-importance"] = "4";
		style["text-fill"] = "#000";
		style["text-halo-fill"] = "#fff8";
		style["text-halo-radius"] = "2.5";
		LayerDef layerDef;
		layerDef.push_back(3);
		layerDef.push_back(1);

		styleDefOut.push_back(StyleAndLayerDef(layerDef, style));
		return 1;
	}

	return 0;

}

int ColourStringToRgba(const char *colStr, double &r, double &g, double &b, double &alpha)
{
	if(colStr[0] == '\0')
		return 0;

	if(colStr[0] == '#')
	{
		if(strlen(colStr) == 7)
		{
			string sr(&colStr[1], 2);
			string sg(&colStr[3], 2);
			string sb(&colStr[5], 2);
			unsigned int tmp;
			sscanf(sr.c_str(), "%x", &tmp);
			r = (double)tmp / 0xff;
			sscanf(sg.c_str(), "%x", &tmp);
			g = (double)tmp / 0xff;
			sscanf(sb.c_str(), "%x", &tmp);
			b = (double)tmp / 0xff;
			alpha = 1.0;
			return 1;
		}

		if(strlen(colStr) == 9)
		{
			string sr(&colStr[1], 2);
			string sg(&colStr[3], 2);
			string sb(&colStr[5], 2);
			string sa(&colStr[7], 2);
			unsigned int tmp;
			sscanf(sr.c_str(), "%x", &tmp);
			r = (double)tmp / 0xff;
			sscanf(sg.c_str(), "%x", &tmp);
			g = (double)tmp / 0xff;
			sscanf(sb.c_str(), "%x", &tmp);
			b = (double)tmp / 0xff;
			sscanf(sa.c_str(), "%x", &tmp);
			alpha = (double)tmp / 0xff;
			return 1;
		}

		if(strlen(colStr) == 4)
		{
			string sr(&colStr[1], 1);
			string sg(&colStr[2], 1);
			string sb(&colStr[3], 1);
			unsigned int tmp;
			sscanf(sr.c_str(), "%x", &tmp);
			r = (double)tmp / 0xf;
			sscanf(sg.c_str(), "%x", &tmp);
			g = (double)tmp / 0xf;
			sscanf(sb.c_str(), "%x", &tmp);
			b = (double)tmp / 0xf;
			alpha = 1.0;
			return 1;
		}

		if(strlen(colStr) == 5)
		{
			string sr(&colStr[1], 1);
			string sg(&colStr[2], 1);
			string sb(&colStr[3], 1);
			string sa(&colStr[4], 1);
			unsigned int tmp;
			sscanf(sr.c_str(), "%x", &tmp);
			r = (double)tmp / 0xf;
			sscanf(sg.c_str(), "%x", &tmp);
			g = (double)tmp / 0xf;
			sscanf(sb.c_str(), "%x", &tmp);
			b = (double)tmp / 0xf;
			sscanf(sa.c_str(), "%x", &tmp);
			alpha = (double)tmp / 0xf;
			return 1;
		}

	}
	return 0;
}

