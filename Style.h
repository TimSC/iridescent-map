#ifndef _STYLE_H
#define _STYLE_H

#include "cppo5m/OsmData.h"

typedef std::map<std::string, std::string> StyleAttributes;

class Style
{
public:
	enum FeatureType {
		Poi,
		Line,
		Area
	};
		

	void GetStyle(int zoom, const TagMap &tags, FeatureType featuretype, StyleAttributes &styleOut);

};

#endif //_STYLE_H
