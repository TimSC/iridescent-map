#ifndef _STYLE_H
#define _STYLE_H

#include "cppo5m/OsmData.h"

typedef std::map<std::string, std::string> StyleAttributes;

class Style
{
public:
	void GetStyle(int zoom, TagMap &tags, StyleAttributes &styleOut);

};

#endif //_STYLE_H
