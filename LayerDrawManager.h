#ifndef _LAYER_DRAW_MANAGER_H
#define _LAYER_DRAW_MANAGER_H

#include <set>
#include "Regrouper.h"

class LayerDrawManager
{
protected:
	void ExtractLayerFromTags(const TagMap &tags, std::set<int> &layerSet);
	
public:
	LayerDrawManager();
	virtual ~LayerDrawManager();

	std::set<int> FindLayers(class FeatureStore &store);

};

#endif //_LAYER_DRAW_MANAGER_H
