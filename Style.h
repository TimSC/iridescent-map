#ifndef _STYLE_H
#define _STYLE_H

#include "cppo5m/OsmData.h"

///Attributes to define how a feature is drawn. Based on CartoCSS attributes.
typedef std::map<std::string, std::string> StyleAttributes;

///A series of indicies that specify a layer, sub-layer, sub-sub-layer, etc.
typedef std::vector<int> LayerDef;

///A set of style tags at a particular (sub-)layer
typedef std::pair<LayerDef, StyleAttributes> StyleAndLayerDef;

///Style definition for a particular tag group, at a particular zoom
typedef std::vector<StyleAndLayerDef> StyleDef;

///Simple storage of a style definition
class Style
{
public:
	enum FeatureType {
		Poi,
		Line,
		Area
	};
	
	int GetStyle(int zoom, const TagMap &tags, FeatureType featuretype, StyleDef &styleDefOut);
};

int ColourStringToRgba(const char *colStr, double &r, double &g, double &b, double &alpha);

#endif //_STYLE_H
