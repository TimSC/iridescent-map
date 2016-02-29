#ifndef _STYLE_H
#define _STYLE_H

#include "cppo5m/OsmData.h"

typedef std::map<std::string, std::string> StyleAttributes;
typedef std::vector<int> LayerDef;
typedef std::pair<LayerDef, StyleAttributes> StyleAndLayerDef;
typedef std::vector<StyleAndLayerDef> StyleDef;

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

#endif //_STYLE_H
