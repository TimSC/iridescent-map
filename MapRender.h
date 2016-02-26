#ifndef _MAP_RENDER_H
#define _MAP_RENDER_H

#include "drawlib/drawlib.h"
#include "Regrouper.h"
#include "Transform.h"

class MapRender
{
public:
	class IDrawLib *output;

	MapRender(class IDrawLib *output);
	virtual ~MapRender();

	void Render(int layerNum, class FeatureStore &featureStore, class ITransform &transform);
};

#endif //_MAP_RENDER_H
