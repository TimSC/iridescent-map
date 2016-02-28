
#include "Style.h"
#include <iostream>
using namespace std;

void Style::GetStyle(int zoom, const TagMap &tags, FeatureType featuretype, StyleAttributes &styleOut)
{
	styleOut.clear();

	TagMap::const_iterator it = tags.find("highway");

	if(it != tags.end())
	{
		styleOut["line-color"] = "#ededed";
	}


}
